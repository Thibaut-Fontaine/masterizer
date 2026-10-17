# Project brief — Masterizer

Living handoff document. It records **what the product is**, **what the code actually
does today**, **what the Mix Advisor is for**, and **what the JUCE VST still needs**.
Code and tests remain authoritative; this file is the map.

Last updated: 2026-10-17.

## Status snapshot

- Core engine, offline CLI and UI logic are complete and tuned on one real pair
  (`PREMASTER.wav` → `MASTER.wav`).
- **278 test cases**, 4 CTest suites (`unit`, `style_rules`, `includes`, `portability`).
- Backlog steps 0–8 are done, plus soft/loud delivery profiles
  (`--profile soft|loud|both`).
- Performance: ~27–35 s per 4.5-minute track for one profile; ~45 s for both.
- **The Mix Advisor (R1–R11) is now surfaced as read-only Source Notes** in the CLI and
  carried by the dashboard presenter. This is the original "fix the mix before mastering"
  feature. The editor rendering is still to come.
- **The pre-VST foundation is complete:** stateful block processors, `RealtimeChain`, the
  VST-facing parameter ids, dashboard fields and `Docs/REALTIME_PLAN.md`.
- **No plugin code exists yet.** `src/plugin/` does not exist and JUCE is not in the
  build. JUCE 8.0.15 is pinned; the VST3 shell is the next major work (Gate 2).
- Gate 1 was signed off by the user's listening verdict on the real pair
  ("the program is good, it does good masters"). Breadth validation is deferred to
  Gate 3; see "Gates" below.

---

## 1. What the project is

A reference-driven automatic mastering VST for electronic music. The user loads a
finished reference track; the plugin analyses the **Reference** and the **DAW
Source**, proposes a coordinated mastering chain, and renders the Source toward the
Reference's tonal balance, perceived density, transients, stereo image and loudness.

Two principles that are easy to lose:

- The Reference is a **target profile, not a command**. The engine may correct less,
  do nothing, or report that a target cannot be reached safely.
- **True Peak = hard constraint, LUFS = soft target, crest/LRA = guards, width =
  optional/diagnostic.** A lower spectral distance is not proof of better sound.

The original user request, in full:

> Analyse the unmastered track and a reference mastered track, generate a mastered
> track from the unmastered one. But after analysing, if the analysis finds mix
> problems (for example, the kick is not loud enough), suggest that the user modify
> the mix of the song before mastering.

That second half is the **Mix Advisor**; it must be visible in the final product.
It is advice only and never gates or delays the automatic master.

## 2. Requirements (non-negotiable)

- **Strict TDD**: failing test first, red → green, then commit and push.
- Every C++ file: **≤ 5 functions**; every function **≤ 60 lines**; every line
  **≤ 110 columns**. `scripts/check_rules.py` enforces this as the `style_rules`
  CTest test. Plugin files are scanned too (`include`, `src`, `tests`).
- **Windows 10 1607+ x64** with Visual Studio 2022 is the primary target; Linux and
  macOS must keep building. VST3 only (no VST2 headers available).
- In-repo, dependency-free test harness. Do not vendor Catch2/doctest/etc.
- Domain code is JUCE-free and fully unit-tested; JUCE adapters live in
  `src/plugin/**` and must not contain DSP logic.
- The audio callback must never allocate, lock, log, decode or do disk I/O.
- Cross-platform must always work on Windows and Linux.
- The tape stage is subtle and must stay optional/toggleable in the VST.
- Do not over-saturate. Prefer signal integrity and report shortfalls.
- Do not retain full decoded reference audio in memory; cache versioned analysis
  profiles and invalidate them when analysis behaviour changes.

## 3. Locked decisions

- C++20, **JUCE 8.0.15** (pinned), CMake ≥ 3.24, **VST3**.
- Layered MVC + Ports & Adapters:
  - Model: `include/masterizer/domain/**`, `src/domain/**`.
  - Controller: `include/masterizer/application/**`, `src/application/**`.
  - View/adapters: `src/plugin/**` (future).
- Metric roles: True Peak hard constraint (default −1.0 dBTP); LUFS soft target;
  crest/LRA guards; width optional/diagnostic.
- Crest guard stops before global crest falls more than **0.5 dB** below the
  reference, reports the shortfall; `--no-crest-guard` opts out.
- Air/high shelf is at **8 kHz** (the measured gap was 8–12 kHz, not >12 kHz).
- Fine EQ is implemented but **opt-in** (`--fine-eq`) until listening validates it.
- Default ceiling stays −1.0 dBTP; `loud` uses −0.1 dBTP.
- The VST exposes the delivery profile as **one `MasterProfile` control**
  (soft/loud), not two separate controls.

## 4. Gates

- **Gate 0 — engine built + unit/integration tested.** Done.
- **Gate 1 — the engine makes sensible sound on real music.** Signed off by the
  user's listening verdict on the Dax J pair (2026-10-17). The formal breadth matrix
  (subgenres, mono/anti-phase/silence, unreachable targets) is deferred to Gate 3.
- **Gate 2 — plugin/host integration** (JUCE VST3, decode adapter, background
  analysis, real-time chain, dashboard, Source Notes). **Next.**
- **Gate 3 — release quality** (validation matrix, pluginval, real DAW, packaging,
  licences).

Do not reopen DSP tuning on the Dax J track; the risk is overfitting. New DSP work
should be driven by the breadth validation or by a concrete plugin requirement.

---

## 5. The mastering pipeline, exactly as coded

### 5.1 Read

`io::readAudioFile` reads WAV (16/24-bit PCM, 32-bit float), FLAC or MP3 into planar
float channels. The CLI writes a 16-bit PCM WAV. The eventual plugin uses the host's
buffer format.

### 5.2 Analyse Source and Reference

`analyzeSource` runs both files in parallel and extracts:

- STFT: symmetric Hann, FFT **2048**, hop **1024** in the CLI (library default 512),
  averaged over frames.
- Ten octave bands: Sub <45, Low 45–90, ULow 90–180, LowMid 180–360, Mid 360–720,
  UMid 720–1440, HMid 1440–2880, Presence 2880–5760, High 5760–11520,
  VHigh >11520 Hz.
- Fine averaged magnitude spectrum (for EQ).
- Per-band variation (short-term dynamics), transient density/crest, resonance
  persistence/peakiness.
- Envelope: peak, RMS, crest, transient density, percussive-prominence estimate.
- Stereo: per-band Mid/Side, true per-band L/R correlation, width.
- BS.1770-4 gated integrated LUFS; True Peak via 4× oversampling.

**Whole-track only.** There is no section detection and no manual section selection
(roadmap Step 9 remains open).

### 5.3 Compare

`compareProfiles` normalises each profile to its own total energy, so a louder
Reference cannot bias the tonal comparison. It produces per-band relative-level
difference (Source − Reference), plus loudness, transient, width and True Peak
differences.

### 5.4 Matching Engine

`MatchingEngine::propose` currently proposes **EQ only**:

- ignore |difference| < **0.5 dB**;
- confidence = |difference| / **6 dB**, reject below **0.2**;
- correction = −difference, clamped to **±6 dB**.

`ChainCoordinator` (max **6** proposals, max **12 dB** total) exists and is tested
but **is not called** by `process()` yet.

### 5.5 Dynamics planner

`planDynamics`:

- Mean per-band variation gap > **0.05** → gentle glue: threshold −18 dB, ratio
  1.5–2.0, attack 20 ms, release 150 ms (or BPM-based), knee 6 dB.
- Gap > **0.2** → also multiband (2-band, LR4 at 200 Hz).
- Upper-mid (bands 5–7) variation gap > **0.05** → dynamic EQ at **3 kHz**, Q 1,
  threshold −18 dB, max cut **2 dB**.

### 5.6 Chain order

`ProcessingChain.cpp` applies, in order, each module bypassable:

1. Mono/M-S low end (95 Hz, non-strict by default)
2. Saturation (off by default)
3. De-resonator (off by default)
4. Static EQ from proposals (peaking Q 1.4; Sub = 40 Hz low shelf;
   VHigh = 8 kHz high shelf; Match Intensity scales amounts, clamped ±6 dB)
5. Fine third-octave EQ (off by default; `--fine-eq`, max 6 filters,
   100 Hz–10 kHz, Q 2, min 1 dB)
6. Stereo width (off by default; a single width scale; never auto-enabled)
7. Transient shaper (off by default)
8. Glue compressor (planner)
9. Multiband compressor (planner)
10. Dynamic EQ (planner)
11. Tape: CLI sets drive **1.0 dB**, head bump **+0.4 dB @70 Hz**, HF shelf
    **−0.1 dB @14 kHz**, mix **0.5**
12. Output limiter — the chain-level limiter is disabled in the CLI; the loudness
    targeter applies the look-ahead limiter at the end.

### 5.7 Loudness targeting and True Peak

`targetLoudness`:

- raise gain toward the Reference integrated LUFS, capped at **+12 dB**;
- up to **5 iterations**, 0.05 LU tolerance, 0.9 relaxation;
- look-ahead limiter: **5 ms** look-ahead, 100 ms release;
- static True Peak guard after the limiter, so the render is at/below the ceiling;
- **crest guard**: stop before global crest falls more than **0.5 dB** below the
  Reference; report the remaining shortfall rather than flattening the track;
- report applied gain, achieved LUFS, True Peak and shortfall.

### 5.8 Delivery profiles

| Profile | Ceiling | Use |
|---|---:|---|
| `soft` (default) | −1.0 dBTP | safe for lossy encoding |
| `loud` | −0.1 dBTP | matches hot references; little codec headroom |

Only the ceiling differs; the chain is identical. `--profile both` shares one
analysis and one chain pass, then renders both and writes `<out>.soft.wav` and
`<out>.loud.wav`.

### 5.9 Optional monitoring files

- `--ab`: level-matched source, gain = source LUFS − achieved LUFS.
- `--delta`: processed − source (not gain-compensated; the chain is zero-latency
  today).

### 5.10 What is actually enabled by default

The code contains more modules than a default render uses.

| Stage | Default in CLI |
|---|---|
| Mono/M-S | **on**, 95 Hz |
| Static EQ | **on** (confidence-gated proposals) |
| Glue / multiband / dynamic EQ | **on only if `planDynamics` fires** |
| Tape | **on** (settings above) |
| Look-ahead limiter + TP guard | **on** (inside targeter) |
| Crest guard | **on** |
| Fine EQ | off (`--fine-eq`) |
| Saturation | off |
| De-resonator | off |
| Stereo width | off (and auto-disabled for near-mono references) |
| Transient shaper | off |

Audible default master: **mono low end → tonal EQ → (planner dynamics) → tape →
loudness/limiter/TP**. Saturation, de-resonance, stereo and transient shaping are
built and tested but not currently proposed automatically.

---

## 6. CLI reference

```text
masterizer_cli <reference> <source> <output.wav> [options]
```

Inputs: WAV / FLAC / MP3 (any mix). Output: 16-bit WAV.

| Option | Meaning | Default |
|---|---|---|
| `--profile soft\|loud\|both` | delivery profile | `soft` |
| `--intensity N` | Match Intensity 0..2 | `1.0` |
| `--ceiling dB` | explicit True Peak ceiling; error with `both` | profile value |
| `--gain dB` | extra/starting gain passed to the targeter | `0` |
| `--ab` | write level-matched `<out>.ab.wav` | off |
| `--delta` | write `<out>.delta.wav` | off |
| `--timing` | per-phase timings | off |
| `--crest-guard` / `--no-crest-guard` | enable/disable the crest guard | on |
| `--eq` / `--no-eq` | static EQ | on |
| `--fine-eq` / `--no-fine-eq` | fine third-octave EQ | off |
| `--mono` / `--no-mono` | low-end mono | on |
| `--limiter` / `--no-limiter` | limiter in targeter | on |
| `--saturation` / `--no-saturation` | saturation | off |
| `--tape` / `--no-tape` | tape stage | on (CLI) |
| `--resonance` / `--no-resonance` | de-resonator | off |
| `--stereo` / `--no-stereo` | stereo width | off |
| `--transient` / `--no-transient` | transient shaper | off |
| `--glue` / `--no-glue` | glue compressor | planner |
| `--multiband` / `--no-multiband` | multiband | planner |
| `--dynamic-eq` / `--no-dynamic-eq` | dynamic EQ | planner |
| `--help`, `-h` | usage | — |

### Known CLI issues

- **Numeric-flag leak (bug):** `--intensity` and `--ceiling` write their parsed
  value into `outputGainDb` while parsing and never restore it. A run with
  `--intensity 0.5` therefore also passes +0.5 dB as `extraGainDb`; `--ceiling -2`
  passes −2 dB. It did not show in the dual-profile run because no numeric flags
  were used. Fix under TDD before the VST.
- **`--gain` is not a clean output trim.** It is passed as `extraGainDb` into the
  targeting loop, which may still raise gain to reach the Reference LUFS. The spec
  wants output gain separate from loudness targeting.
- `--ceiling` with `--profile both` is rejected (both profiles would become
  identical), by design.
- Match Intensity currently scales **EQ only**; the other modules ignore it.

---

## 7. Mix Advisor — the original "fix the mix first" requirement

`advise()` is a second consumer of the same Source/Reference comparison. It never
modifies audio and never gates or delays the automatic master. A stereo master
cannot identify instruments, so every suggestion is hedged:

- never assert an instrument as fact;
- emit a **ranked plausible-source list** + confidence + "verify by ear";
- use frequency **×** temporal structure (percussive vs sustained) and persistence,
  not frequency alone;
- keep magnitudes small and reversible;
- stay silent when evidence is weak;
- level-normalise Source and Reference before comparing.

### Rules (data table, `AdvisorRuleset`, version 1)

| Rule | Condition | Suggested source action | Likely sources | Master-fixable |
|---|---|---|---|---|
| R1 low-end punch | Sub/Low variation gap < −0.15 | +1–2 dB on kick, slow attack, carve bass | kick (likely), bass masking | partly |
| R2 low-end width/phase | correlation < 0.3 and width gap > 0.15 | mono below 90–100 Hz, check bass patch | wide sub/bass, stereo kick | yes |
| R3 source over-compression | ref crest − source crest ≥ 4 dB | back off mix-bus limiter, keep headroom, re-bounce | mix-bus limiter/compressor | no |
| R4 low-mid buildup | band 3 energy gap ≥ 2.5 dB | gentle source cut 200–300 Hz or carve bass | bass, keys/guitars, kick body | partly |
| R5 missing air | min(band 8, band 9 energy gap) < −2.5 dB | lift air/presence at source; check low-pass/monitoring | dark source, low-pass | yes |
| R6 harsh presence | band 7 energy gap ≥ 2.5 dB and density gap ≥ 0.5 | tame 3–5 kHz, soften attack | hats/cymbals, lead | yes |
| R7 narrow image | ref width − source width ≥ 0.15 | widen pads/keys/reverb; check stereo bus | over-monoed mix | yes |
| R8 dull high transients | min(band 7, band 8 density gap) < −0.5 | reduce limiting/compression on highs | over-limited cymbals/overheads | no |
| R9 resonant bass | band 0–2 peakiness > 8 and persistence > 0.4 | dynamic EQ/notch at source | resonant bass/synth, room mode | yes |
| R10 kick/bass conflict | Sub or Low energy gap ≥ 2.5 dB and band 1 density gap < −0.5 | sidechain or carve 60–100 Hz | kick + bass | partly |
| R11 phase problem | correlation ≤ −0.2 | check polarity on that source | polarity/phase issue | yes |

### Current status and gaps

- **`advise()` is now called by the CLI and carried by the dashboard presenter.** The
  editor still has to render the list; there is no JUCE code yet.
- Tests cover identical profiles, R3, R5, R7 and R11; there is no fixture per rule.
- `MixSuggestion` now carries `rulesetVersion`, and `adviseFeedback()` provides the
  `MixFeedback { rulesetVersion, suggestions[] }` wrapper. `actionTemplateId` is still
  absent; action text is hard-coded in the rules.
- There is no cap/de-duplication pass across rules.
- The spec requires a read-only "Source notes" list with plain-text export, showing
  the band/feature, plain-language action, confidence and "verify by ear".

---

## 8. What the VST must add (JUCE)

The core is an **offline, whole-buffer** engine. A VST needs a real-time,
block-based path and background analysis.

### 8.1 Plugin shell

- `PluginProcessor`, `PluginEditor`, `juce_add_plugin`, VST3, stereo I/O, bypass,
  state save/restore, latency reporting.
- Expose the existing parameter model: master enable, Match Intensity, ceiling,
  output gain, module enables. Add `MasterProfile` (soft/loud), `FineEqEnabled`,
  `TapeEnabled`, `CrestGuardEnabled`, BPM.
- The tape stage stays optional/toggleable.

### 8.2 Reference workflow

- File chooser + drag-and-drop; `JuceAudioDecoder` implementing `IAudioDecoder`.
- Background decode → `analyzeSource` → cache the **profile**, then drop the audio.
- Status: unloaded / loading / ready / stale / failed; missing-file warning on
  project restore.
- Use the existing `ReferenceManager` and `ProfileCache`, but note:
  `ReferenceManager` currently keeps the full decoded reference buffer in memory,
  which violates the "do not retain full decoded reference audio" rule. The plugin
  should keep only the profile.

### 8.3 Source analysis

- The DAW gives blocks, not a file. Options: capture a representative section,
  capture a whole pass, or incremental analysis.
- For the first VST, a **bounded capture/analysis pass** is the honest approach:
  play the track (or a selected section), accumulate analysis, then apply. The spec
  calls this "Analyze & Apply" and allows manual section selection.
- Analysis and chain planning run off-thread; hand the immutable profile/chain
  settings to the audio thread safely.
- Do not allocate/copy a full 4.5-minute stereo buffer in the audio callback.

### 8.4 Real-time chain

- The **offline** DSP functions create filter/envelope state **inside each call**. They
  are correct for the CLI but would reset state if called per host block.
- The **stateful processors now exist** (`EqProcessor`, `MonoBassProcessor`,
  `CompressorProcessor`, `TransientProcessor`, `DynamicEqProcessor`, `TapeProcessor`,
  `MultibandProcessor`, `LookaheadProcessor`), and `RealtimeChain` assembles them in the
  offline module order with per-channel state.
- `RealtimeChain` is block-size independent, allocates in construction (not in
  `process`), applies gain before the look-ahead limiter, and reports latency.
- The plugin still has to build the plan (bands + gain) from the analysis, hand it to
  the audio thread, and crossfade/rebuild safely when it changes.
- Loudness targeting cannot be a closed loop over the future in real time. Natural
  VST design: during analysis, compute the required gain + chain settings; then
  apply a fixed gain and a real-time look-ahead limiter at the chosen ceiling.
  Show achieved LUFS/shortfall as a meter. True two-pass limiting is not
  universally available in VST3.
- **True Peak gap:** `LookaheadProcessor` limits sample peak. The offline path has a
  4× oversampled True Peak guard. Before release, either oversample the limiter
  detector or add a final True Peak safety stage; do not label the sample-peak ceiling
  as True Peak until then.

### 8.5 Monitoring

- A/B: dry vs processed, with monitoring gain = source LUFS − processed LUFS, kept
  separate from output gain.
- Delta: processed − dry, latency-compensated if the real-time chain adds delay.
- The current look-ahead limiter is zero-latency gain anticipation, so no delay
  today; if we add true look-ahead delay, report it to the host.

### 8.6 Dashboard and Source Notes

- Octave-band Source vs Reference, difference, correction, module activity,
  confidence.
- LUFS, True Peak, shortfall, warnings.
- A read-only **"Source notes"** panel fed by `advise()`: action, confidence,
  "verify by ear", never an unhedged instrument claim.
- The existing `DashboardModel` is a good start but only carries band
  levels/difference/correction, LUFS, TP and warnings. It needs suggestions, module
  activity, confidence, profile and analysis state.

### 8.7 Wiring gaps to fix while building the VST

1. ~~Wire `advise()` into the CLI/dashboard~~ — **done** (Source Notes).
2. Wire `ChainCoordinator::coordinate` into `process()` — **deliberately deferred**; it
   would change the approved master.
3. ~~Fix the CLI numeric-flag leak~~ — **done** (`--intensity`/`--ceiling` no longer leak
   into `outputGainDb`).
4. Make output gain a true final trim, separate from loudness targeting — **pending**;
   it changes the approved master, so it needs an explicit decision.
5. Make Match Intensity scale the other modules, not just EQ — **pending**; it changes
   the sound and needs listening validation.
6. ~~Add stateful block processors for the real-time chain~~ — **done**
   (`RealtimeChain`).
7. Consider section analysis (roadmap Step 9) — still absent; deferred with a plan.
8. Add 24-bit/32-float WAV export at some point (currently 16-bit only).
9. Stereo-link the dynamics modules (currently per-channel, image-shift risk).
10. Use `ChainReporter` and `OutputValidator` in the production path (currently only
    tests / inline report).

### 8.8 Proposed plugin structure (for approval, not created)

```text
src/plugin/
  PluginProcessor.h/.cpp        host I/O, bypass, latency, state, parameter attachment
  PluginEditor.h/.cpp           top-level editor layout
  ParameterLayout.h/.cpp        APVTS layout from domain/state/Parameters + profile
  JuceAudioDecoder.h/.cpp       IAudioDecoder via JUCE AudioFormatManager
  ReferenceController.h/.cpp    load / clear / replace / status / cache
  AnalysisWorker.h/.cpp         background analysis + chain planning
  SourceCapture.h/.cpp          bounded capture / streaming accumulation
  RealtimeChain.h/.cpp          stateful block DSP (EQ, mono, dynamics, tape, limiter)
  DashboardComponent.h/.cpp     octave bars + meters
  SourceNotesComponent.h/.cpp   Mix Advisor suggestions
  PluginState.h/.cpp            reference metadata + profile cache + parameters
```

Build plan:

- Pin **JUCE 8.0.15** (`git ls-remote` confirms the tag exists).
- `juce_add_plugin(Masterizer ... FORMATS VST3)`, link `masterizer_core`.
- `MASTERIZER_BUILD_PLUGIN` option; keep the core test build JUCE-free.
- Linux CI can install the missing ALSA/freetype/X11 dev packages; this sandbox
  cannot (no passwordless sudo), so the actual plugin build/load test happens on
  Windows/MSVC or CI.
- All plugin files still obey ≤5 functions, ≤60 lines/function, ≤110 columns.

---

## 9. Work state (checkpoint, reconciled)

### Completed

- Measurement, analysis, Matching Engine, Mix Advisor (R1–R11), full bypassable
  chain, coordination module, output validation, A/B + Delta, dashboard presenter
  and parameter model.
- Performance: 18 min 11 s → ~27–35 s per 4.5-minute track (~39×); regression
  contract held.
- Backlog steps 0–8:
  - 0 diagnostic (`Docs/DIAGNOSTIC_DAXJ.md`)
  - 1 crest guard + `--no-crest-guard`
  - 2 edge-band shelves (Sub 40 Hz low shelf, VHigh 8 kHz high shelf)
  - 3 tape HF roll-off reduced (−0.1 dB @14 kHz, mix 0.5)
  - 4 third-octave fine EQ (opt-in `--fine-eq`)
  - 5 conditional upper-mid dynamic EQ (3 kHz, ≤2 dB)
  - 6 true per-band L/R correlation
  - 7 near-mono stereo guard (<0.05 width)
  - 8 look-ahead limiter
- HF gap fixed: 7.9 kHz +1.47 → +0.41 dB, 10 kHz +1.93 → −0.28 dB,
  High octave +0.65 → +0.16 dB.
- Dual delivery profiles (`--profile soft|loud|both`), 278 tests green.
- Pre-VST foundation: Mix Advisor Source Notes, CLI numeric-flag fix, VST-facing parameter
  ids, dashboard profile/confidence/module fields, stateful block processors,
  `RealtimeChain`, and `Docs/REALTIME_PLAN.md`.
- All commits pushed to `origin/main`.

### Active

- Listening comparison of `DUO.soft.wav` (−11.53 LUFS, −1.00 dBTP) and
  `DUO.loud.wav` (−10.53 LUFS, −0.10 dBTP) against `MASTER.wav`.
- User verdict: little audible difference apart from `soft` being slightly less
  loud; the engine produces sensible masters.
- `OURS8.wav` / `OURS9.wav` are the earlier equivalents of `DUO.soft` / `DUO.loud`
  and are superseded but kept for continuity.

### Blocked

- The JUCE VST3 shell cannot be compiled in this sandbox (missing Linux GUI/ALSA/
  freetype development libraries; no passwordless sudo). It needs Windows/MSVC or
  installing the Linux deps.
- The user has no OpenCode access on Windows; `Docs/WINDOWS_TESTING.md` gives
  standalone build/test instructions for the current core + CLI.

### Next move (ordered)

1. Gate 2: JUCE VST3 shell in small buildable steps (pass-through shell → decoder/reference
   workflow → background analysis → plan handoff → dashboard → Source Notes).
2. Extend Match Intensity to the other modules (changes sound; needs a listening decision).
3. Gate 3: breadth validation, pluginval, real DAW, packaging, licences.
4. Section analysis (roadmap Step 9) remains deferred with a test plan.
5. Do not tune further on the Dax J track (overfitting risk).

`ChainCoordinator` and a true final output trim are deliberately not wired: both would change
the approved master. Treat them as sound decisions, not refactors.

### Relevant files

- `Docs/BACKLOG.md` — live todo, metric roles, speed plan S0–S7.
- `Docs/PRODUCT_SPEC.md` — binding requirements and validation matrix.
- `Docs/ROADMAP.md` — required implementation order (22 steps / stages 0–10).
- `Docs/DIAGNOSTIC_DAXJ.md` — step-0 diagnostic and HF follow-up.
- `Docs/REFLECTIONS.md` — per-stage reflections, performance and HF lessons.
- `Docs/ARCHITECTURE.md` — layering and contracts.
- `Docs/DSP_DEFINITIONS.md` — measurement/processing definitions.
- `Docs/TESTING.md` — test harness and coverage.
- `Docs/WINDOWS_TESTING.md` — Windows build/test instructions.
- `Docs/REALTIME_PLAN.md` — real-time threading, capture, handoff, latency and TP plan.
- `app/main.cpp` — CLI pipeline.
- `src/application/ProcessingChain.cpp` — ordered chain.
- `src/application/RealtimeChain.cpp` — block-based chain assembly.
- `src/domain/dsp/*Processor.cpp` — stateful real-time processors.
- `src/application/LoudnessTargeter.cpp` — LUFS targeting, crest guard, limiter.
- `src/domain/matching/EqDesigner.cpp` — shelf placement (8 kHz air).
- `src/domain/matching/MatchingEngine.cpp` — tonal proposals.
- `src/domain/matching/DynamicsPlanner.cpp` — glue/multiband/dynamic EQ.
- `src/domain/matching/MixAdvisor.cpp` + `Rules*.cpp` — R1–R11.
- `src/domain/analysis/SourceAnalyzer.cpp` — parallel analysis.
- `src/io/AudioFile.cpp` — WAV/FLAC/MP3 input.
- `include/masterizer/application/DashboardModel.h` — UI presenter.
- `include/masterizer/domain/state/Parameters.h` — parameter ids.

---

## 10. Important context and numbers

### Reference pair

| File | LUFS | True Peak | Sample peak | RMS | Crest | Format / length |
|---|---:|---:|---:|---:|---:|---|
| `MASTER.wav` | −10.50 | +0.12 dBTP | −0.50 dBFS | −11.24 dBFS | 10.74 dB | 24-bit/44.1 kHz, 270.00 s |
| `PREMASTER.wav` | −17.99 | −3.83 dBTP | — | — | 14.98 dB | 269.76 s |
| `OURS8` / `DUO.soft` | −11.53 | −1.00 dBTP | −1.62 dBFS | −12.29 dBFS | 10.67 dB | 269.76 s |
| `OURS9` / `DUO.loud` | −10.53 | −0.10 dBTP | −0.72 dBFS | −11.30 dBFS | 10.58 dB | 269.76 s |

`DUO.loud` is closest to `MASTER`; both were produced in 44.6 s from one analysis.

### Findings that must not be forgotten

- The online comparison the user shared earlier was invalid: the "before" file was
  447.53 s, not the 269.8 s `PREMASTER`.
- The HF gap was at **8–12 kHz**, not >12 kHz. The 12 kHz shelf under-corrected it;
  moving the shelf to 8 kHz fixed it.
- Saturation/exciter evidence is weak. Do not add saturation on spectral evidence
  alone.
- Default ceiling remains −1.0 dBTP per spec; `loud` is the explicit −0.1 dBTP
  profile.
- `PREMASTER.wav`, `MASTER.wav`, `OURS*.wav`, `DUO*.wav` are git-ignored. Do not
  commit them.

### Recent commits (before this brief)

`6503d40` dual profiles · `bc33b7c` HF shelf 8 kHz · `1def764` docs · `2d22e8c`
`--no-crest-guard` · `cc91ace` fine EQ · `3915ce3` look-ahead limiter · `b70e530`
per-band correlation · `a07b49e` dynamic EQ · `9f2ed15` stereo guard · `e131405`
tape HF · `4195c68` edge shelves · `4806c65` crest guard.

Pre-VST work: `3db66be` Source Notes · `48f6138` CLI numeric fix · `b4c4dda` parameter
ids · `0961333` dashboard fields · `75c2705` stateful processors · `b0b2c76`
`RealtimeChain`, plus per-step reflections.

Test count before the pre-VST work: **244 cases**. Current: **278 cases**, 4 CTest suites green.
