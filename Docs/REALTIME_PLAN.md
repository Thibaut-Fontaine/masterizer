# Real-time plan — from the offline engine to the VST

Design note for Gate 2. It defines how the tested offline core becomes a safe VST3:
thread boundaries, data handoff, capture strategy, latency and monitoring. It is a
plan; the plugin code does not exist yet.

## 1. Hard constraints

- The audio callback (`processBlock`) must not allocate, lock, log, decode or do disk I/O.
- Analysis and chain planning are offline/background work. They never run on the
  audio thread.
- `AnalysisProfile` and the planned chain are immutable once published.
- The plugin must survive variable block sizes, sample-rate changes, bypass, project
  restore, silence, mono/anti-phase input and a missing reference file.
- Every parameter/state change must be safe under concurrent UI, host automation and
  audio processing.

## 2. Threads and ownership

```text
Message/UI thread
  - file chooser / drag-and-drop
  - parameter edits (APVTS)
  - status, dashboard repaint
  - starts/stops a capture session
        |
        v
Analysis worker (JUCE ThreadPool / background thread)
  - decode reference (JuceAudioDecoder -> IAudioDecoder)
  - analyzeSource(reference) -> immutable profile
  - analyzeSource(captured source) -> immutable profile
  - propose chain + Mix Advisor feedback
  - build a RealtimePlan (settings + resolved bands + gain + latency)
        |
        v
Atomic/shared_ptr handoff (no locks on the audio thread)
        |
        v
Audio thread
  - reads the current plan at a block boundary
  - runs RealtimeChain::process(left, right, n)
  - meters output level / true peak
```

The audio thread never waits for the worker. Until a plan is ready it passes the
source through unchanged (or runs the previous plan).

## 3. Reference workflow

1. User drops/selects a file.
2. Message thread records `path`, size and modification time (`ReferenceKey`), shows
   `Loading`.
3. Worker decodes through `IAudioDecoder` (JUCE `AudioFormatManager` adapter; the same
   port the tests fake).
4. Worker validates the decode, runs `analyzeSource`, and stores the profile in
   `ProfileCache` keyed by `ReferenceKey`.
5. Worker publishes `shared_ptr<const AnalysisProfile>` and shows `Ready`.
6. **Drop the decoded audio.** Only the profile is retained (spec/AGENTS rule).
7. If the file is missing or changed on project restore, show `Stale` / `Failed` and
   keep the last profile read-only; never crash and never silently analyse a different
   file.

Cache invalidation: `ProfileVersion` (currently 2.0) must be part of the cache key or
checked on lookup, exactly as `ProfileCache` already does.

## 4. Source analysis

The DAW supplies blocks, not a file. Two modes, in order:

### 4.1 Analyze & Apply (first VST)

- The user arms a capture and plays a representative section (or the whole track).
- The audio thread writes blocks into a **preallocated** single-producer /
  single-consumer FIFO. No allocation in the callback.
- The worker drains the FIFO into a bounded capture buffer, then runs `analyzeSource`.
- Proposed default capture: **60 s** stereo at the host sample rate (~23 MB at 48 kHz),
  with a hard cap (for example 5 minutes / ~115 MB) and a clear "capture full length"
  option. A full 10-minute capture is not kept by default.
- After analysis, the worker builds the plan and publishes it. The user applies it
  (transport stopped) or the plugin crossfades to it over a few milliseconds.

A whole-track capture is optional; if added, it must be bounded and documented.
This is the honest compromise for a stereo master plugin that cannot read the DAW's
source file.

### 4.2 Live Match (later)

- Accumulate analysis statistics over a rolling window and refresh proposals at a
  slow control rate (e.g. every 1–2 s), not per frame.
- Never let live meters drive processor parameters per sample. The plan changes at
  block boundaries, with smoothing/crossfade.
- The spec marks Live Match as optional; Analyze & Apply is the required path.

## 5. Plan construction

The worker turns the two profiles into a `RealtimePlan`:

```text
RealtimePlan {
  ChainSettings settings;          // module enables + parameters
  RealtimeChainBands bands;        // resonance, static EQ, fine EQ
  double preLimiterGainDb;         // loudness gain, bounded
  int latencySamples;              // from the limiter
  MasterProfile profile;           // soft / loud
  MixFeedback advice;              // read-only Source notes
  ProfileVersion sourceVersion, referenceVersion;
}
```

Construction reuses the tested offline functions:

- `compareProfiles` -> `MatchingEngine::propose` -> `designEq` (and Match Intensity
  scaling in `scaleEqAmount`).
- `detectResonances` for the de-resonator bands (when enabled).
- `designFineEq` for the opt-in fine EQ.
- `planDynamics` for glue / multiband / dynamic EQ.
- `coordinate` is **not applied by default**: it would change the approved master.
  If enabled later, it needs an explicit A/B decision.
- `adviseFeedback` for Source notes. It is never part of the render chain.

Parameter changes (Match Intensity, module enables, profile, ceiling) rebuild the plan
off-thread. The audio thread only swaps to a fully constructed plan.

## 6. Loudness in real time

The offline targeter measures the whole render and iterates; a VST cannot see the
future. The real-time strategy is:

1. During analysis, compute source integrated LUFS and reference integrated LUFS.
2. Set `preLimiterGainDb = clamp(referenceLufs - sourceLufs, 0, +12)`.
3. Apply the same crest guard rule as the offline path: if the projected crest falls
   more than 0.5 dB below the reference, reduce the gain and record a shortfall.
4. In the audio thread, apply the fixed gain and then the look-ahead limiter at the
   ceiling selected by `MasterProfile` (soft −1.0 dBTP, loud −0.1 dBTP).
5. Meter the actual output integrated LUFS (short-term) and report the remaining
   shortfall. Do not adapt the gain in a feedback loop.

The result may differ slightly from the offline render because the real-time chain
cannot iterate; the UI must show the measured value, not claim equality.

**True Peak gap:** `LookaheadProcessor` limits the sample peak. The offline path has a
4× oversampled True Peak guard. Before release, either (a) oversample the limiter
detector, or (b) add a final True Peak safety stage after the limiter. Do not call the
sample-peak ceiling "True Peak" in the UI until this is closed.

## 7. Latency, A/B and Delta

- `RealtimeChain::latencySamples()` returns the limiter look-ahead (240 samples at
  48 kHz / 5 ms by default). Report it with `setLatencySamples`.
- The dry A path must be delayed by the same amount so A/B is aligned.
- Delta = processed (delay-compensated) − dry. It reveals changes, not whether they
  sound better; say so in the UI.
- Monitoring gain = source LUFS − processed LUFS. It is separate from the exported
  gain and must never change the render.
- If the chain adds more latency later, compensate every monitoring path and update
  the reported value.

## 8. State and parameters

- Host parameters (APVTS) come from `domain/state/Parameters`: master enable,
  `MasterProfile` (0 = soft, 1 = loud), Match Intensity, ceiling, output gain,
  limiter, EQ, fine EQ, mono, saturation, tape, stereo, transient, glue, multiband,
  dynamic EQ, resonance, crest guard.
- Persist: parameter values, reference path + `ReferenceKey`, the selected profile,
  A/B/delta state, capture mode, and the cached profile (or a serialized profile with
  its `ProfileVersion`).
- On restore: if the reference file is missing, show the failure and keep the last
  cached profile; do not block audio and do not crash.
- Automation: read parameter atomics in `processBlock`; rebuild plans off-thread and
  crossfade. No hard parameter jumps on audible controls.

## 9. Failure modes to handle explicitly

- Silence / near-silence: analysis returns floors; plan should be a no-op, not NaN.
- Mono input, anti-phase input, very wide input: no width chasing; keep mono
  compatibility.
- Very quiet / very loud input: gain is bounded; report shortfall.
- Unsupported/missing/changed reference: error state; no silent substitution.
- Sample-rate change: rebuild every processor; reset filter/limiter state.
- Block sizes from 1 to at least 2048: `RealtimeChain` is block-size independent;
  `MultibandProcessor::prepare(maxBlockSize)` is called in `prepareToPlay`.
- Bypass: crossfade or hard bypass; never leave stale limiter gain.
- Offline render: no real-time assumptions; the chain must work with large blocks.
- NaN/inf: sanitize and report; never propagate.

## 10. Validation plan (Gate 3)

- Unit/integration: already in repo (278 cases, block-size equivalence, ceiling).
- Host: `pluginval` at strictness 5+; at least one real DAW.
- Sample rates: 44.1, 48, 88.2, 96 kHz.
- Block sizes: 32, 64, 128, 256, 512, 1024, 2048.
- Scenarios: bypass, automation, project save/restore, missing reference, changed
  reference, mono/anti-phase/silence, offline render, CPU/memory.
- Listening: level-matched A/B across subgenres; do not treat a lower spectral
  distance as proof of better sound.

## 11. Build and verification route

- JUCE 8.0.15 pinned; `juce_add_plugin(... FORMATS VST3)`.
- `MASTERIZER_BUILD_PLUGIN` option; the core test build stays JUCE-free.
- This sandbox cannot `apt install` (no passwordless sudo), but `apt-get download` +
  `dpkg-deb -x` into a local sysroot is a viable way to compile/link the VST3 here.
- CI (Ubuntu + Windows) installs dependencies and builds the VST3; Windows/MSVC is the
  primary target and produces the artifact for real DAW testing.
- The editor itself remains untested until a human opens it in a DAW.

## 12. Open decisions

1. Capture length: 60 s default vs full-track opt-in; memory cap.
2. Whether to serialize analysis profiles to disk, or only cache them in the project
   state.
3. True Peak limiting: oversampled detector vs final safety stage.
4. Plan switching: apply only when stopped vs short crossfade during playback.
5. Stereo-linked dynamics: keep per-channel (matches the offline render) or add a
   link option (changes the sound; needs listening).
6. Whether `ChainCoordinator` is ever enabled; it would change the approved master.
