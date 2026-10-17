# VST/JUCE build plan — simple UI first

Gate 2 plan. Companion to `Docs/REALTIME_PLAN.md` and `Docs/PROJECT_BRIEF.md`.
**Status: proposal. No plugin code exists yet.**

## Goal

A working VST3 that:

1. loads a finished reference track and analyses it in the background,
2. captures/analyses the DAW source,
3. builds the same bounded mastering plan as the offline engine,
4. processes the source in real time through `RealtimeChain`,
5. lets the user A/B, monitor Delta, switch soft/loud and bypass, and
6. shows a **simple** dashboard plus read-only Mix Advisor "Source notes".

The approved mastering chain and thresholds must not change. The plugin reuses the
offline plan; it does not invent a new one.

## Non-goals for this pass

- Fancy visual design, custom look-and-feel, animation, OpenGL.
- Per-band manual EQ editing, drag-to-shape curves, preset browser.
- Stem/source separation, instrument identification, ML.
- Live Match mode (Analyze & Apply is the required path).
- Linear-phase EQ, oversampling modes, multiband UI editing.
- macOS/Linux packaging (Linux builds are for CI; Windows is the release target).

## Working rules

- One buildable milestone at a time; green tests, commit, push, reflection.
- Domain/application logic stays JUCE-free and unit-tested. The editor is a thin
  render layer over `DashboardModel`.
- The audio callback never allocates, locks, logs, decodes or does disk I/O.
- Every file still obeys ≤5 functions / ≤60 lines per function / ≤110 columns.
- Label sample-peak limiting honestly until True Peak is handled.
- Do not tune the master while building the shell. If a plugin bug suggests a DSP
  change, stop and treat it as a separate listening decision.

---

## M0 — Build plumbing

**Goal:** a pinned JUCE VST3 target that compiles and links in CI and, if possible,
in this sandbox, without disturbing the core build.

- [ ] **M0.1** Decide JUCE acquisition. Preferred: `FetchContent` with
      `GIT_TAG 8.0.15`. Add a `MASTERIZER_JUCE_DIR` cache variable so an existing
      checkout can be used offline on Windows/CI.
- [ ] **M0.2** Add root option `MASTERIZER_BUILD_PLUGIN` (default OFF). When ON,
      include JUCE and `add_subdirectory(src/plugin)`.
- [ ] **M0.3** Add `src/plugin/CMakeLists.txt` with `juce_add_plugin(Masterizer ...)`:
      `FORMATS VST3`, `IS_SYNTH FALSE`, `NEEDS_MIDI_INPUT FALSE`,
      `NEEDS_MIDI_OUTPUT FALSE`, `COPY_PLUGIN_AFTER_BUILD TRUE`,
      `VST3_CATEGORIES Fx`, company/product ids.
- [ ] **M0.4** Link `masterizer_core`; add `juce::juce_recommended_config_flags`,
      `juce::juce_recommended_lto_flags` (Release), and warning flags that do not
      fight the repo's `-Wall -Wextra -Wpedantic`.
- [ ] **M0.5** Add a compilable skeleton `PluginProcessor` (stereo buses) and
      `PluginEditor` (generic editor placeholder) so the target links.
- [ ] **M0.6** Add a CI `plugin` job for Ubuntu and Windows: install the Linux dev
      packages, configure with `-DMASTERIZER_BUILD_PLUGIN=ON`, build Release, upload
      the VST3 artifact. Keep the existing core jobs unchanged.
- [ ] **M0.7** Add `scripts/fetch_plugin_deps.sh` for the sandbox: `apt-get download`
      + `dpkg-deb -x` the ALSA/freetype/X11/GL dev packages into a local sysroot, then
      print the `CMAKE_PREFIX_PATH`/`PKG_CONFIG_PATH` to use. Keep the sysroot
      git-ignored.
- [ ] **M0.8** Verify: core configure/build/tests with plugin OFF; plugin configure/
      build with plugin ON in CI and (if the sysroot works) locally.
- [ ] **M0.9** Add a short "plugin build" section to `Docs/WINDOWS_TESTING.md`.
- **Acceptance:** plugin target configures, builds and links; core tests still pass
  with the plugin OFF; CI is green.
- **Tests:** build only.
- **Commit:** `build: pinned JUCE VST3 target`.

## M1 — Minimal pass-through VST3

**Goal:** a host-correct stereo pass-through shell.

- [ ] **M1.1** `PluginProcessor` bus layout: stereo in/out; reject unsupported
      layouts; `acceptsMidi`/`producesMidi` false; `getTailLengthSeconds` 0.
- [ ] **M1.2** `prepareToPlay` stores sample rate and max block size; `releaseResources`
      resets state; no allocation in `processBlock`.
- [ ] **M1.3** `processBlock`: `ScopedNoDenormals`; clear unused output channels; copy
      input to output; handle an empty/silent block safely.
- [ ] **M1.4** Bypass: the host bypass must produce dry audio; the plugin's own
      "master enabled" parameter comes in M2.
- [ ] **M1.5** `createEditor` returns `juce::GenericAudioProcessorEditor`; `hasEditor`
      true.
- [ ] **M1.6** Add `tests/plugin` target (built only when `MASTERIZER_BUILD_PLUGIN`
      is ON): instantiate the processor, prepare at 48 kHz/512, process silence and a
      sine, assert finite and unchanged; check bypass.
- [ ] **M1.7** Run `pluginval` at strictness 5 (or the highest it passes); fix crashes
      and bus/state warnings before adding DSP.
- [ ] **M1.8** Manual Windows DAW load: play, seek, bypass, offline render.
- **Acceptance:** VST3 loads, passes audio, bypass works, no clicks, no crashes on
  prepare/release, pluginval baseline recorded.
- **Tests:** headless processor smoke test + pluginval + DAW checklist.
- **Commit:** `feat(plugin): minimal pass-through VST3`.

## M2 — Parameter layout and state

**Goal:** every user-facing control exists, is automation-safe and restores.

- [ ] **M2.1** `ParameterLayout`: map `domain/state/Parameters` to APVTS.
      Use `AudioParameterChoice` for `MasterProfile` ("Soft", "Loud"), bool parameters
      for enables, floats for intensity/ceiling/gain with ranges from `specOf`.
- [ ] **M2.2** Add a test that APVTS ids/ranges/defaults exactly match
      `allParameters()` + `specOf`; catch drift immediately.
- [ ] **M2.3** Cache `std::atomic<float>*` pointers for audio-thread reads; no string
      lookups or `getParameter` calls in `processBlock`.
- [ ] **M2.4** Parameter listeners set a plan-dirty atomic; do not rebuild in the
      audio callback.
- [ ] **M2.5** State: `getStateInformation`/`setStateInformation` via APVTS
      `copyState`/`replaceState`; add a `stateVersion` property; ignore unknown/invalid
      state safely.
- [ ] **M2.6** Headless tests: parameter count/ids/ranges/defaults, state round-trip,
      choice values, automation clamping.
- [ ] **M2.7** Confirm the generic editor still exposes all controls.
- **Acceptance:** complete parameter list; defaults match the CLI sound; state restore
  in a fresh instance; automation does not glitch.
- **Tests:** APVTS/state unit tests; pluginval.
- **Commit:** `feat(plugin): parameter layout and state`.

## M3 — Reference workflow

**Goal:** load a reference, analyse it off-thread, cache the profile, never keep the
decoded audio.

- [ ] **M3.1** `JuceAudioDecoder` implements `IAudioDecoder` with
      `juce::AudioFormatManager` (WAV/FLAC/MP3/AIFF as available); return a populated
      `DecodedAudio` or a clear error.
- [ ] **M3.2** `ReferenceController` state machine: Unloaded / Loading / Ready /
      Stale / Failed; `load(path)`, `unload()`, `isStale(key)`, error text.
- [ ] **M3.3** `ReferenceKey` from path + size + modification time; compare on project
      restore.
- [ ] **M3.4** Use `ReferenceManager` + `ProfileCache`; analyse on the analysis
      worker; drop the decoded buffer after the profile exists.
- [ ] **M3.5** File chooser (async `juce::FileChooser`) and editor drag-and-drop
      (`FileDragAndDropTarget`).
- [ ] **M3.6** UI: "Load reference" button, name, status, error/progress.
- [ ] **M3.7** Tests: `ReferenceController` with a fake `IAudioDecoder` for success,
      failure, missing and stale cases; no JUCE file I/O in the unit test.
- [ ] **M3.8** Manual: load WAV/FLAC/MP3, invalid file, missing file, project restore.
- **Acceptance:** a real reference loads/analyses without blocking audio; errors are
  visible and safe.
- **Tests:** controller unit tests + manual file checks.
- **Commit:** `feat(plugin): reference loading and analysis worker`.

## M4 — Source capture and analysis

**Goal:** capture a bounded source section and analyse it off-thread.

- [ ] **M4.1** `SourceCapture`: preallocated SPSC ring/FIFO; `prepare(sampleRate,
      maxBlock, seconds)`; `push(left,right,n)` from the audio thread; `drain` on the
      worker. No allocation in `push`.
- [ ] **M4.2** Capture policy: default 60 s, hard cap 5 min, documented; state machine
      Idle/Capturing/Analysing/Ready/Failed; reset/cancel.
- [ ] **M4.3** Analysis worker: one background thread with a job queue; progress
      callbacks to the UI via `AsyncUpdater`/`callAsync`, never from the audio thread.
- [ ] **M4.4** Analyse the captured buffer with `AnalysisSettings` derived from the
      host rate; build the Source `AnalysisProfile`.
- [ ] **M4.5** Build `MixFeedback` with `adviseFeedback` (Source notes).
- [ ] **M4.6** UI: "Analyze" / capture progress in seconds / cancel / status.
- [ ] **M4.7** Tests: FIFO across block boundaries, cap enforcement, silence,
      anti-phase, mono; no allocation in push.
- [ ] **M4.8** Manual: capture a real section, verify the profile and notes.
- **Note:** a captured section is compared to the **whole-track reference** in v1.
  Show a warning that the comparison is approximate. Full reference-section matching
  is a later feature.
- **Acceptance:** capture → analyse works on a real track; silence/edge cases safe.
- **Tests:** FIFO/capture unit tests + manual.
- **Commit:** `feat(plugin): source capture and analysis`.

## M5 — Plan construction and handoff

**Goal:** turn the profiles into an immutable plan and process it in real time.

- [ ] **M5.1** Add JUCE-free `application/RealtimePlan.h`:
      `ChainSettings`, `RealtimeChainBands`, `preLimiterGainDb`, `latencySamples`,
      `MasterProfile`, `MixFeedback`, source/reference profile versions.
- [ ] **M5.2** Add `application/PlanBuilder.h/.cpp`: `MatchingEngine::propose` →
      `designEq` + Match Intensity scaling; `planDynamics`; `detectResonances`;
      `designFineEq`; `adviseFeedback`. No audio.
- [ ] **M5.3** Gain/crest calibration: compute a candidate gain from source vs
      reference LUFS, then run a **bounded preview** of the captured section through
      a `RealtimeChain` to measure achieved LUFS and crest; adjust the gain once
      (or twice), clamp to +12 dB, and record the predicted shortfall. This replaces
      the live closed loop without lying about the target.
- [ ] **M5.4** Handoff: publish an immutable `shared_ptr<const RealtimePlan>`.
      Validate `std::atomic<std::shared_ptr<...>>` support in M0; if unavailable,
      use a preallocated double-buffer or a `try_lock` on a spinlock (never block
      the audio thread). Document the chosen mechanism.
- [ ] **M5.5** Build `RealtimeChain` off-thread. For v1, apply the new plan when
      transport is stopped; if live switching is added, crossfade over 20–50 ms.
- [ ] **M5.6** `processBlock`: read the current plan, run `RealtimeChain`, report
      latency, respect bypass; pass dry audio until a plan exists.
- [ ] **M5.7** Plan-dirty triggers: reference ready, source ready, profile/intensity/
      module/ceiling change. Coalesce rebuilds; never rebuild per block.
- [ ] **M5.8** Tests: `PlanBuilder` gain/crest bounds, band assembly, profile
      mapping; plan-swap smoke; latency; block-size independence.
- **Acceptance:** processed audio, no clicks at plan swap, latency reported, bypass
  restores dry audio, block-size independent.
- **Tests:** JUCE-free `PlanBuilder` tests + plugin smoke.
- **Commit:** `feat(plugin): plan construction and real-time handoff`.

## M6 — Simple dashboard UI

**Goal:** a usable one-page editor. Ship the smallest useful UI first, then add the
secondary controls.

### UI cut 1 — must have

- [ ] **M6.1** `PluginEditor` one-page layout, dark neutral background, resizable
      900×600, stock JUCE controls.
- [ ] **M6.2** `HeaderComponent`: reference load/name/status, profile combo,
      Match Intensity slider, Analyze/Apply, master enable/bypass.
- [ ] **M6.3** `BandMeterComponent`: 10 octave groups; Source bar, Reference bar,
      difference text; no animation. Only custom drawing on the page.
- [ ] **M6.4** `MeterComponent`: LUFS, True Peak, shortfall, warning text. Update on a
      message-thread timer (10–20 Hz), never from audio.
- [ ] **M6.5** `MonitorComponent`: A/B buttons, Delta toggle, monitoring-gain label.
- [ ] **M6.6** `SourceNotesComponent`: read-only multiline text, collapsible, empty
      state "No source notes.", always "verify by ear".
- [ ] **M6.7** `DashboardModel` feed: build `DashboardInput` on the message thread from
      processor snapshots (profiles, proposals, report, suggestions, modules).
- [ ] **M6.8** Editor smoke/manual checklist: load, capture, apply, A/B, bypass, notes;
      no manual required.
- [ ] **M6.9** Confirm no DSP or analysis calls from the editor.

### UI cut 2 — after cut 1 works

- [ ] **M6.10** `ModuleTogglesComponent`: compact wrapped toggles for EQ, fine EQ,
      mono, limiter, tape, crest guard, glue, multiband, dynamic EQ, saturation,
      stereo, transient, resonance.
- [ ] **M6.11** Advanced panel: ceiling override, output gain, monitoring gain value,
      capture length, "full track" opt-in.
- [ ] **M6.12** Planner-driven module indicators (enabled/reason) next to the toggles.

- **Acceptance (cut 1):** a human can load a reference, capture, apply, A/B, bypass
  and read Source Notes without a manual.
- **Tests:** presenter already tested; manual UI checklist; add an editor smoke test
  only if headless testing is reliable.
- **Commit:** `feat(plugin): simple dashboard editor`.

## M7 — A/B, Delta and monitoring gain

- [ ] **M7.1** Dry delay line of `latencySamples`, preallocated; A path aligned with B.
- [ ] **M7.2** A/B state with click-free switching (short crossfade or aligned hard
      switch); B = processed, A = delayed dry.
- [ ] **M7.3** Delta = processed (delay-compensated) − dry; expose it on the monitor
      path only.
- [ ] **M7.4** Monitoring gain = source LUFS − processed LUFS; apply to the monitored
      processed signal, never to the exported audio.
- [ ] **M7.5** Tests: impulse alignment, identical-signal delta ≈ 0, monitoring gain
      does not change render, click-free switch.
- [ ] **M7.6** UI copy: "Delta reveals changes, not whether they sound better."
- **Acceptance:** aligned, level-matched A/B; Delta contains only the change.
- **Commit:** `feat(plugin): A/B, Delta and monitoring gain`.

## M8 — True Peak and limiter hardening

- [ ] **M8.1** Choose and document: oversampled limiter detector **or** a final True
      Peak safety stage.
- [ ] **M8.2** Implement the chosen approach; keep the output gain order
      (gain → limiter) documented.
- [ ] **M8.3** Test 44.1/48/88.2/96 kHz and block sizes 32–2048; verify the ceiling
      with a 4× True Peak measurement, not sample peak.
- [ ] **M8.4** Flush or document the limiter tail at stream end.
- [ ] **M8.5** Recheck `setLatencySamples` and A/B/Delta compensation.
- [ ] **M8.6** UI labels "True Peak" only after the proof.
- **Acceptance:** output at/below the selected ceiling by a documented method.
- **Commit:** `feat(plugin): true-peak-safe output stage`.

## M9 — Hardening and host validation

- [ ] **M9.1** Automated plugin tests: sample rates, block sizes, mono, silence,
      anti-phase, NaN/inf, bypass, plan swap.
- [ ] **M9.2** `pluginval` strictness 5+ on Windows; record the log.
- [ ] **M9.3** Real DAW checklist: load, play, seek, bypass, automation, save/restore,
      offline render, missing reference.
- [ ] **M9.4** CPU/memory check; verify no allocation in `processBlock` (debug
      allocator/instrumentation).
- [ ] **M9.5** State/cache invalidation on `ProfileVersion` change.
- [ ] **M9.6** Record results in `Docs/VST_VALIDATION.md`; no unverified claims.
- **Acceptance:** validation matrix recorded; no crashes, NaNs or unexpected gain.
- **Commit:** `test(plugin): host validation matrix`.

## M10 — Release packaging

- [ ] **M10.1** Windows build script and VST3 install/copy instructions.
- [ ] **M10.2** Versioning (`JucePlugin_VersionString`) and release notes.
- [ ] **M10.3** CI artifacts for Windows and Linux.
- [ ] **M10.4** Licence review: JUCE, VST3, dr_libs.
- [ ] **M10.5** Update `README`, `AGENTS.md`, `Docs/WINDOWS_TESTING.md`, known
      limitations.
- [ ] **M10.6** Reproducible build check from a clean checkout.
- **Acceptance:** versioned, installable VST3 with documented limitations.
- **Commit:** `chore: VST3 release packaging`.

---

## Simple UI rules

- One page, no tabs. Dark neutral background, two accent colors, system font.
- Stock `juce::Label`, `Slider`, `ComboBox`, `TextButton`, `ToggleButton`,
  `TextEditor`.
- No custom look-and-feel, images, animation or OpenGL.
- The octave view is the only custom drawing: 10 groups of two bars plus text.
- One component per file to satisfy the ≤5-functions rule.
- Numbers: LUFS 0.1, dB 0.1, intensity 0–200%.
- Warnings are plain text, not modal dialogs (except load failures).
- Never block the message thread on analysis; show "Analysing…".
- Keyboard focus order must be sane; every control has a visible label.
- Cut 1 ships before cut 2. Do not gold-plate.

## Proposed plugin structure

```text
src/plugin/
  CMakeLists.txt
  PluginProcessor.h/.cpp
  PluginEditor.h/.cpp
  ParameterLayout.h/.cpp
  JuceAudioDecoder.h/.cpp
  ReferenceController.h/.cpp
  AnalysisWorker.h/.cpp
  SourceCapture.h/.cpp
  PlanHandoff.h/.cpp
  PluginState.h/.cpp
  ui/HeaderComponent.h/.cpp
  ui/BandMeterComponent.h/.cpp
  ui/MeterComponent.h/.cpp
  ui/MonitorComponent.h/.cpp
  ui/SourceNotesComponent.h/.cpp
  ui/ModuleTogglesComponent.h/.cpp
```

JUCE-free additions:

```text
application/RealtimePlan.h
application/PlanBuilder.h/.cpp
```

## Testing strategy

- **Core/application:** normal in-repo tests, no JUCE, no host.
- **Plugin headless tests:** parameter layout, state round-trip, `prepareToPlay` /
  `processBlock` with silence/sine, bypass, plan swap, FIFO/capture. No editor, no
  audio device.
- **Editor:** manual checklist; a headless smoke test only if reliable.
- **`pluginval`:** Windows + Linux CI where possible.
- **Real DAW:** Windows, at least one host; load, play, seek, save/restore, offline
  render, bypass.
- **Listening:** level-matched A/B across subgenres; never use "lower spectral
  distance" as proof.

## Risks / blockers

- **Sandbox cannot build JUCE yet.** No passwordless sudo; Docker socket denied.
  The deb-sysroot script is the fallback; CI is the reliable path. Validate in M0.
- **Headless JUCE tests may need a message/GUI initialiser.** Keep them minimal; if
  they are flaky, rely on pluginval for the shell and keep logic in JUCE-free code.
- **Threading** is the main correctness risk: one worker, one immutable plan, no
  blocking in `processBlock`.
- **True Peak** is the main quality risk: the real-time limiter is sample-peak only.
- **Capture UX** is unusual: 60 s default, 5 min cap, section-vs-whole-reference
  warning.
- **`≤5 functions` with JUCE components:** split early.
- **Open-loop loudness** can differ from the CLI; the preview calibration in M5.3 is
  the mitigation, and the UI must show measured values, not promises.

---

## Reflection on this plan

**What is strong**

- The order is dependency-driven: build → shell → parameters → reference → capture →
  plan → UI. Each milestone is independently verifiable and can be stopped.
- The risky parts (threading, True Peak, capture UX) are named before coding.
- The UI is split into cut 1 and cut 2, which keeps "simple first" honest.
- The plan reuses the already-tested offline logic and the `RealtimeChain`; it does
  not reopen the approved mastering sound.

**What is weak or risky**

- **M0 is still unproven.** The local deb-sysroot may fail on transitive JUCE deps;
  CI iteration is slower. This is the first thing to validate, not the last.
- **M3/M4 are the hardest milestones**, not M5. A capture FIFO, a background worker,
  progress reporting and no audio-thread allocation is real concurrency work. The
  plan may need an extra day and a dedicated stress test.
- **M5's preview calibration is an approximation.** Running the real-time chain on a
  captured section estimates the gain, but the section may not represent the whole
  track. The UI must label it as a prediction, not a guarantee.
- **Section vs whole-track matching is a conceptual gap.** A 60 s source section is
  compared to a whole-track reference; a verse-only capture can bias the tonal
  comparison. The warning helps, but a later reference-section selector is the real
  fix.
- **The plan-swap mechanism is undecided.** `std::atomic<std::shared_ptr>` support
  must be checked; otherwise the fallback needs a careful preallocated double-buffer.
  Applying only on stop is simpler but limits the workflow.
- **M6 is still a lot of UI** even in cut 1. The band view, meters, transport and
  notes are four components; keep the drawing minimal or the milestone will drift.
- **M8 can change the sound.** An oversampled limiter detector is not a free
  refactor; it may alter peaks and latency. Treat it as a listening decision.
- **The plan assumes one source section is enough.** Real tracks change; a single
  capture can miss the loudest/brightest section. Consider "capture full track"
  earlier if the user's workflow allows it.

**What I would change before starting**

1. Keep M0 + M1 as the first stop; do not start M2 until the plugin loads in a DAW.
2. Add a tiny concurrency stress test in M4 (many blocks, odd sizes, cancel) before
   wiring the UI progress.
3. Decide the plan-swap mechanism in M0 with a compile spike, not in M5 under
   pressure.
4. Ship UI cut 1 without module toggles; put them in cut 2.
5. Keep the captured-section warning visible from the first capture onward.

**What would invalidate the plan**

- JUCE cannot be built/loaded in the available environment at all.
- Host block/state behaviour cannot be tested reliably.
- The capture UX is unacceptable to the user (then full-track capture or manual
  section selection becomes the critical path).
- True Peak oversampling changes the approved sound more than expected.

**The one thing to validate first:** M0 + M1 — a pinned JUCE pass-through VST3 that
loads in a DAW with `pluginval` clean. Everything else is cheaper to design once that
is real.
