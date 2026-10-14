# Product Spec — Reference-Based Automatic Mastering VST

> Binding product requirements. Formerly the root `AGENTS.md`. Agent-facing
> entry point and working rules live in `/AGENTS.md`.

## 1. Mission

Build a reference-driven automatic mastering plugin primarily for electronic music. The user loads a finished reference track; the plugin analyses the Reference and the DAW Source, proposes a coordinated mastering chain, and processes the Source to approach the Reference's relevant tonal balance, perceived density, dynamics, transient/percussive prominence, stereo image and loudness.

The Reference is a **target profile, not a command to damage the Source**. The product must try to get closer to the target while preserving the Source's musical character. It must be able to leave a characteristic unchanged, use a lighter correction, or report that a target cannot safely be reached.

The original product concept must not be diluted into a generic EQ-matching plugin. Keep the following ideas central: octave-by-octave Source-versus-Reference analysis; Youlean-inspired multiband metering; percussive prominence; low-end mono/M-S management below roughly 90–100 Hz; optional overall drive; multiband harmonic saturation; de-resonance; stereo matching; glue and multiband compression; dynamic EQ; True Peak limiting; Match Intensity; A/B and Delta monitoring.

## 2. Core product requirements — preserve these ideas

### 2.1 Octave-first analysis and UI

Use the following bands as the primary UI and summary representation:

- Sub: `<45 Hz`
- Low: `45–90 Hz`
- Upper low: `90–180 Hz`
- Low-mid: `180–360 Hz`
- Mid: `360–720 Hz`
- Upper mid: `720–1,440 Hz`
- High-mid: `1,440–2,880 Hz`
- Presence: `2,880–5,760 Hz`
- High: `5,760–11,520 Hz`
- Very high: `>11,520 Hz`

The two outer bands are edge bands, not complete octaves. For complete interior octave bands, label geometric centres accurately (for example, the centre of 180–360 Hz is approximately 255 Hz, not 220 Hz). Do not preserve incorrect example labels just because they appeared in early sketches.

Octaves are the main conceptual and visual model, but must **not** be the only internal analysis resolution. Use finer logarithmic bins, a suitable STFT representation, or an equivalent approach for narrow resonances and precise EQ. Where useful, expose both the broad octave summary and finer spectral detail.

### 2.2 Youlean-inspired multiband meters

Build the metering internally. Use Youlean-style loudness/dynamics visualization as a product reference, not as a required external plugin or assumed DSP dependency.

For each octave band, show Source, Reference target, measured difference and relevant processing activity for:

- Band energy/density (at minimum RMS or mean-square energy).
- Peaks, crest factor and transient/envelope behaviour.
- Stereo width and Mid/Side energy.
- Estimated percussive/transient prominence.
- Applied correction and, where possible, confidence/uncertainty.

Show standardized full-signal LUFS, loudness over time and True Peak separately. Do not label ordinary per-band RMS as standardized LUFS. If implementing an additional band-loudness metric inspired by loudness weighting, document its exact calculation and label it clearly as a custom metric.

To be clear, the idea is NOT to display anything to the user. It is to take in consideration metrics for actually building the masterization.

### 2.3 Drums versus sustained instruments

Keep the original goal of comparing drum/percussive prominence with sustained or harmonic content. Estimate it from transient density, attack strength, duration, crest factor, envelope shape, and transient-versus-sustained energy by frequency region. Give special attention to kick/snare-relevant regions without claiming that frequency alone identifies an instrument.

A stereo master does not reliably identify or isolate every kick, snare, bass, synth or other source. The baseline feature is an estimate of **percussive prominence**, not guaranteed source separation. Add more advanced separation or stem-aware processing only as a later capability. Use transient shaping when justified; avoid increasing every sharp synth/pluck just because the reference has stronger drums.

### 2.4 Sub mono and overall drive

Include low-frequency mono/M-S management with a default transition region around 90–100 Hz. Support gradual Side reduction, configurable transition/slope and a user-selectable strict mono mode. Compare the effect in stereo and in mono. Do not force mono summation on every Source, and do not claim that it alone guarantees club or vinyl compatibility.

Preserve the original idea of an optional **overall drive** control, in addition to optional multiband saturation. It may be proposed when useful, but harmonic-spectrum differences alone do not prove that saturation is necessary. Level-compensate comparisons, bound the drive, and monitor added distortion and peak growth.

### 2.5 Complete reference-led mastering chain

The full product should retain all of these capabilities. They can be implemented in stages; do not silently drop advanced items from the backlog.

1. Input analysis and gain management.
2. Low-end mono/M-S control below approximately 90–100 Hz.
3. Optional global drive/saturation.
4. Smart De-Resonator and corrective EQ.
5. Optional multiband harmonic saturation.
6. Frequency-region EQ/boost and stereo-width matching.
7. Transient shaper based on percussive prominence.
8. Optional gentle global glue compression (typical starting ratio 1.5:1–2:1, not a fixed requirement).
9. Reference-guided multiband dynamics/density control.
10. Fine-resolution/dynamic tonal EQ matching.
11. True Peak maximizer, output gain and export/codec-oriented peak headroom.

The initial canonical processing order may be revised when measurements show that another order works better, but any reordering must be deliberate and documented. Consider interactions between all processors; avoid modules undoing one another.

### 2.6 Tempo-aware compressor release

Offer optional BPM-aware release suggestions when tempo is available and reliable. Support ordinary time-based release as well. Do not force tempo synchronisation; it must be possible to disable it, and it must not override musical or stability constraints or create pumping merely to follow a grid.

### 2.7 Core controls

The UI must include:

- Reference drag-and-drop/file selection, reference status and re-analysis.
- Analyze & Apply workflow and an optional Live Match mode.
- Octave-band Source-versus-Reference dashboard inspired by Youlean-style meters.
- Match Intensity from 0% to 200%.
- Level-matched A/B switching.
- Delta monitoring (processed minus original, latency-compensated).
- Per-module bypass and visible correction amounts.
- Full-signal loudness, True Peak and output status.
- Clear warnings when analysis is incomplete or a requested target cannot be reached safely.

At 0% Match Intensity, automatic matching should be neutral unless the user explicitly enables an independent module. At 100%, apply the engine's proposed corrections. Above 100%, strengthen corrections only through module-appropriate mappings and within safety bounds; never multiply all DSP parameters blindly.

### 2.8 Loudness and output target

Use a configurable True Peak ceiling with `-1.0 dBTP` as the default. Treat reference LUFS as a target, not an unconditional command. Prioritise signal integrity, bounded processing and True Peak control over exact LUFS equality. Report any remaining difference when the reference level cannot be reached without excessive limiting.

Separate the rendered output gain from A/B loudness compensation. The latter must never silently change the exported audio level.

## 3. Technical baseline

- Language: C++20 unless a documented build constraint requires otherwise.
- Framework: JUCE; pin a known-tested release and review licensing for the intended distribution model.
- Build: CMake.
- Initial format: VST3.
- Keep code compatible with the initial target OS and avoid platform-specific assumptions in core DSP. Add other OSes/formats after the first target works.
- Use Git and maintain reproducible Debug/Release builds.
- Keep third-party dependencies minimal and review their licences before distribution.
- Keep the core analysis, matching logic, DSP, plugin-host integration, state and UI separate.
- All DSP algorithms must be testable independently of the DAW and GUI where practical.

## 4. Analysis engine

### 4.1 Reference and Source profiles

Generate structured analysis profiles for both signals. A profile should contain global statistics and, where available, time/section statistics. Cache reusable Reference analysis. Avoid retaining full decoded audio in memory unless a concrete feature requires it.

Measure and document at least:

- Per-band RMS/mean-square energy and relative spectral balance.
- Finer-resolution spectral envelope for EQ matching.
- Peaks, crest factor and envelope dynamics.
- Transient density/prominence and transient-versus-sustained behaviour.
- Full-signal integrated and short-term LUFS using a documented standard implementation/method.
- True Peak using a documented method with appropriate oversampling.
- Mid/Side energy, width indicators and left/right correlation by band.
- Time variation and differences among track sections.
- Candidate narrow resonances or unusually persistent frequency concentrations.
- Optional harmonic/distortion descriptors, explicitly treated as estimates rather than proof of a required saturation treatment.

Use documented window sizes, smoothing, hop sizes, weighting and aggregation. Ensure Source and Reference use identical analysis settings.

### 4.2 Section and arrangement handling

Analyse the whole track and, when feasible, maintain shorter-window statistics or detected sections. Support manually selected representative passages. Allow comparison of comparable drops/sections when their structure permits it.

Do not assume that two globally different arrangements should have identical band-energy profiles. A difference can be caused by a missing instrument, a different section or different sound design rather than a mastering fault. If section matching is unreliable, reduce correction confidence instead of trying harder to force a match.

### 4.3 Measurement definitions

Use standardized full-signal LUFS and True Peak calculations where applicable. For octave-level density, use documented band energy measures such as RMS. If a custom loudness-like band metric is implemented, define it precisely and do not call it standardized LUFS.

Normalise or otherwise account for global level difference before evaluating tonal balance. Do not let a louder Reference automatically trigger a broad EQ boost or increased compression in every band.

For stereo, use a combination of Mid/Side energy, width measures and correlation rather than treating correlation as a complete definition of width. Be careful with anti-phase material and the effect of mono summation.

## 5. Matching Engine and decision logic

### 5.1 Responsibilities

The Matching Engine is the decision-making core. It compares Source and Reference profiles and proposes bounded corrections. Meter readings must not directly drive processor parameter changes frame by frame.

Classify candidate differences into:

- Tonal balance.
- Global and bandwise dynamics.
- Transient/percussive prominence.
- Stereo width/compatibility.
- Potential resonances.
- Global loudness and peak constraints.

Each proposal should include at least: target module, target band, direction, amount, confidence/quality indicator, parameter bounds and a reason or diagnostic label where practical.

### 5.2 Confidence and conservative behaviour

Use stable statistics and confidence estimates. A low-confidence classification should lead to a weaker correction or no correction. Avoid false precision and never claim that a master-only signal reveals the original mix chain or exact sound source.

Apply explicit bounds to EQ boost/cut, dynamic EQ, gain reduction, transient shaping, saturation, stereo width and limiting. Make limits configurable internally and expose only meaningful controls in the UI.

### 5.3 Global optimisation and interaction

Evaluate the cumulative result, not only each module in isolation. Saturation changes spectral content and peak levels; transient shaping changes crest factor; compressors may undo transient shaping; EQ changes detector energy; limiters may erase earlier peak changes; stereo processing changes mono compatibility.

The Matching Engine should optimise a weighted compromise among tonal similarity, dynamics, transient behaviour, stereo image and output level, with a penalty for unnecessary or extreme processing. Do not blindly minimise every metric at the expense of musical quality.

A basic deterministic rule-based engine is acceptable for the MVP. Do not add machine learning unless there is a defined target, training/evaluation data, and evidence that it improves results.

### 5.4 Re-evaluation

After processing, analyse the result or relevant render statistics and confirm whether the intended corrections helped. If iterative optimisation is used, it must be bounded, deterministic where practical, and protected against oscillation. Do not implement an uncontrolled feedback loop driven by live meters.

## 6. DSP module specifications

Implement processors in isolation first, with documented units, parameter ranges, signal-flow assumptions, and tests.

### 6.1 Input gain management

Set a predictable internal operating level where needed without changing the intended output level. Prevent saturation/limiting modules from reacting unpredictably to arbitrary input gain. Do not insert normalisation that silently alters the exported target.

### 6.2 Low-end mono/M-S processor

Provide adjustable transition frequency around 90–100 Hz, slope/crossover behaviour, Side attenuation and strict mono mode. Avoid abrupt spectral/phase changes; test crossover behaviour and summed mono. Make the module bypassable.

### 6.3 Smart De-Resonator

Detect narrow or persistent energy concentrations using level relative to neighbouring bins, bandwidth, persistence and temporal consistency. Use dynamic EQ/notch-like attenuation only when evidence supports it. Avoid treating every tonal peak as an unwanted resonance; synths and basses may be deliberately resonant. Limit attenuation and display the active frequencies/correction.

### 6.4 Overall drive and multiband saturation

Support optional global drive and optional per-band saturation, including selectable saturation character where practical. Monitor level, added harmonics, distortion and peak growth. Compensate output level for comparisons. Do not automatically chase even/odd harmonic ratios as if they identify the precise missing processing chain.

### 6.5 EQ and stereo matching

Keep tonal EQ correction distinct from width correction. Use fine-resolution EQ internally, smoothed targets, limited filter counts and bounded gains. Offer appropriate minimum-phase and, if justified, linear-phase options; document latency and artefact trade-offs. Stereo matching should work by frequency region and be checked against mono compatibility.

### 6.6 Transient shaper

Use transient/envelope statistics and the estimated percussive-prominence gap to choose attack/sustain adjustments. Avoid asserting exact kick/snare detection from frequency bands alone. Ensure sharp synths and plucks are not unintentionally exaggerated. Provide bypass and bounded attack/sustain controls.

### 6.7 Glue compressor

Implement a gentle optional compressor with independently configurable threshold, ratio, attack, release, knee and makeup gain. A ratio near 1.5:1–2:1 is a starting point, not a universal target. Enable it only when justified. Provide optional BPM-aware release recommendations with a manual/non-synchronised mode.

### 6.8 Multiband dynamics/density control

Use a well-defined multiband crossover and independent envelope detectors. The Matching Engine calculates target settings from analysed dynamic differences; normal compressor detectors then react to the audio. Add gain-reduction caps, parameter smoothing and controls against pumping/crossover artefacts. Do not treat an RMS mismatch alone as proof that compression is needed.

### 6.9 Dynamic EQ / tonal matching

Use the fine-resolution profile to calculate smooth corrections. Dynamic EQ may be used for differences that vary over time; static EQ may be more suitable for stable tonal imbalance. Bound boosts/cuts and avoid chasing every short-lived spectral difference. Recheck the cumulative result after upstream saturation and dynamics.

### 6.10 Maximizer and output protection

Provide output gain, configurable True Peak ceiling (default `-1.0 dBTP`) and final loudness measurement. Use suitable oversampling and test the result at supported sample rates. The engine must report target shortfall rather than apply unlimited limiting to hit reference LUFS.

## 7. User interface requirements

### 7.1 Main dashboard

The main dashboard must make the Source-vs-Reference comparison immediately understandable. Show the octave bands as the central view, with measured Source, Reference target, difference, correction and relevant dynamic/stereo indicators. Avoid displaying so many numbers that the main view becomes unusable; provide detailed values in a secondary/advanced view.

Display clearly:

- Analysis state: unanalysed, analysing, ready, stale or failed.
- Reference file/status and selected passage if applicable.
- Match Intensity (0–200%).
- Full-signal loudness and True Peak.
- Per-module on/off state and correction amount.
- A/B and Delta monitoring.
- Warnings for low confidence, target shortfall, missing files and limited analysis.

### 7.2 Reference workflow

Support drag-and-drop and file selection. Show basic metadata (name, duration, sample rate/channels when relevant), analysis status, re-analyse, clear/replace and error states. Never decode files or perform full-track analysis on the audio thread.

### 7.3 A/B, gain matching and Delta

Provide fast A/B switching. Loudness-match monitoring for fair listening, but keep monitoring gain entirely separate from rendered output gain. Align/latency-compensate signals before producing the Delta signal. Document that Delta reveals changes, not whether changes sound better.

### 7.4 Match Intensity

Use module-specific scaling curves. `0%` should disable automatic matching corrections; `100%` applies the normal calculated proposal; `200%` strengthens corrections without bypassing the bounds. Do not linearly multiply every ratio, threshold, time, gain or stereo value.

### 7.5 Advanced controls

Provide per-module bypass and optional advanced controls for users who need manual overrides. The automatic system must not continuously overwrite user-selected parameters. Clearly separate automatic and manual settings when both are supported.

## 8. Architecture and suggested repository structure

Keep the exact file layout flexible, but preserve these component boundaries:

```text
Source/
  PluginProcessor.*       # Host I/O, parameters, state, safe DSP handoff
  PluginEditor.*          # UI
  Analysis/                # Reference/Source feature extraction
  Matching/                # Comparison, confidence, decision and bounds
  DSP/                     # Audio processors
  State/                   # Persistence and reference metadata/cache
Tests/                     # Unit, regression, integration and fixtures
Scripts/                   # Build, test, packaging helpers
Docs/                      # Architecture, DSP definitions, test notes
```

Recommended logical classes/services:

- `ReferenceManager`: load, decode, cache and identify reference files.
- `AnalysisWorker`: background full-track analysis and profile generation.
- `SourceAnalyzer`: real-time-safe statistics capture and/or offline source analysis.
- `AnalysisProfile`: immutable, versioned set of analysis results.
- `MatchingEngine`: profile comparison and bounded correction proposals.
- `ProcessingChain`: coordinated DSP graph and parameter handoff.
- `OutputValidator`: final loudness, True Peak and target-shortfall reporting.
- `PluginProcessor`: host integration, real-time routing and state.
- `PluginEditor`: dashboard and controls.

Use clear data contracts between components. Do not let UI components access DSP internals directly. Use immutable analysis profiles or another race-safe transfer mechanism when handing background results to the audio engine.

## 9. Real-time audio and implementation rules

The audio callback must never perform disk I/O, file decoding, network access, full-track analysis, blocking waits, unbounded memory allocation or verbose logging. Avoid unbounded locks in the audio thread. Prepare expensive resources off-thread and transfer them safely.

Support host variable block sizes, supported sample rates/channel layouts, automation, bypass, state restore and offline rendering. Smooth audible parameter changes. Avoid clicks, NaNs, infinities, denormals, unstable filters and unexpected gain explosions. Handle silence, near-silence and invalid analysis values safely.

If a mode introduces latency, report it to the host and compensate internal A/B/Delta paths. Do not claim zero latency unless measured and verified. Parameter/state updates must be safe under concurrent UI, host automation and audio processing.

Use deterministic behaviour where practical. Analysis results should record their algorithm/profile version so that caches are invalidated when analysis behaviour changes.

## 10. Development roadmap — implement in this order

Do not try to implement the whole product in one change. Each step must build and have a clear definition of done before progressing.

### Stage 0 — Repository reconnaissance and plan

- Inspect the existing repository before changing files.
- Identify OS, compiler, package manager, DAW and current build state if known.
- Choose and pin tested C++/JUCE/CMake versions; document licence implications.
- Create/update README and this file if architecture changes.

**Done when:** dependencies and supported initial target are documented and the build plan matches the repository.

### Stage 1 — Minimal plugin shell

- Create CMake project and VST3 target.
- Implement pass-through stereo audio, bypass and minimal UI.
- Add parameter/state persistence.
- Confirm the plugin loads in the target DAW and survives playback, seek, save/restore and offline render.

**Done when:** a clean Debug and Release build succeeds and the pass-through plugin passes host validation.

### Stage 2 — DSP test harness

- Add tests/utilities for RMS, peak, spectrum, LUFS/True Peak measurement wrappers, Mid/Side and stereo correlation.
- Add synthetic fixtures: silence, impulse, sine, sweep, pink noise, transient-rich signal, bass-heavy signal, wide stereo and anti-phase audio.
- Add safety assertions for finite output and gain bounds.

**Done when:** the DSP test suite runs without a DAW and provides repeatable results.

### Stage 3 — Reference loading and profile cache

- Implement file selection/drag-and-drop, supported format decoding, validation and status UI.
- Run decoding and whole-track analysis on a background worker.
- Cache reference profiles and handle missing/changed files safely.

**Done when:** reference loading never blocks the audio thread and valid profiles can be reused.

### Stage 4 — Analysis MVP

- Implement octave-band energy and finer-resolution spectral profiles.
- Add full-signal LUFS and True Peak measurement, crest factor/envelopes, percussive-prominence features and stereo/M-S statistics.
- Add global and time-window summaries; provide manual section selection where practical.
- Display Source-versus-Reference differences without processing audio.

**Done when:** measurements are documented, repeatable and correctly labelled; the dashboard shows the key original concept.

### Stage 5 — Matching Engine MVP

- Classify tonal, dynamic, transient, stereo and peak/loudness differences.
- Produce correction proposals with confidence, bounds and diagnostics.
- Add conservative/no-op behaviour for uncertain differences.
- Test against identical signals, same-track versions and deliberately dissimilar arrangements.

**Done when:** proposed corrections are visible and reviewable before they are applied.

### Stage 6 — First usable automatic master

- Implement bounded fine-resolution EQ matching.
- Add low-end M/S reduction and optional strict mono mode.
- Add True Peak maximizer/output stage and target-shortfall reporting.
- Add Match Intensity, A/B with independent loudness compensation, latency-aligned Delta and per-module bypass.

**Done when:** the plugin can load a Reference, analyse a Source, apply tonal matching, manage the low end and produce a safely limited output.

### Stage 7 — Dynamic and percussive matching

- Add optional glue compression.
- Add multiband compressor/density control driven by offline or stable analysis proposals.
- Add transient shaper based on percussive-prominence estimates.
- Add optional BPM-aware release suggestions.

**Done when:** dynamics improve on representative tests without systematic pumping or destruction of transients.

### Stage 8 — Resonance, drive and stereo refinement

- Add Smart De-Resonator with conservative detection and visible activity.
- Add optional global drive and multiband saturation with level compensation.
- Improve frequency-region stereo matching and mono checks.
- Add dynamic EQ where differences vary over time.

**Done when:** each advanced module passes independent tests and the combined chain does not introduce obvious regressions.

### Stage 9 — Full-chain coordination

- Integrate modules in the chosen default signal flow.
- Evaluate cumulative chain behaviour and resolve conflicting corrections.
- Apply correction smoothing, safety bounds, stop conditions and output re-analysis.
- Verify Match Intensity scaling for each module.

**Done when:** the chain works as one coordinated system, rather than as independent processors repeatedly undoing one another.

### Stage 10 — Hardening, validation and release

- Run unit, regression and plugin validation tests.
- Test in at least one real DAW; add others based on supported-platform scope.
- Test sample rates, variable block sizes, automation, bypass, project restore, missing references, offline renders and CPU/memory use.
- Run level-matched listening comparisons across electronic subgenres and references with different arrangements.
- Package release builds and document installation, known limitations and licences.

**Done when:** test results are recorded, unresolved limitations are explicit and the release build is reproducible.

## 11. Validation matrix

Always test at least:

- Source identical to Reference (the matching engine should propose little or no correction).
- Same mix before/after mastering, when available.
- Different tracks of the same subgenre.
- Different arrangements where spectral differences should not be overcorrected.
- Bass-heavy techno/house, dense drum programming, transient-heavy material, sustained pads and bright/harsh material.
- Very quiet and very loud input levels.
- Mono, wide-stereo and anti-phase material.
- Silence and near-silence.
- References with much higher loudness than the Source.
- An unreachable loudness target under configured limiting constraints.
- Invalid, unsupported, missing and changed reference files.

Track both objective results (spectral difference, RMS, loudness, peak, gain reduction, mono behaviour, CPU) and listening results. Use level-matched comparisons. A lower numeric distance from the Reference does not, by itself, prove better mastering.

Use `pluginval` or an equivalent plugin-validation tool and at least one real DAW. Never claim a build, test, DAW load or listening test passed unless it was actually run and the result inspected.

## 12. Coding-agent workflow and standards

- Read the relevant code, tests and build instructions before making changes.
- Preserve working behaviour and unrelated code. Do not perform opportunistic refactors.
- Implement one buildable step at a time, following the roadmap unless a concrete blocker requires a documented deviation.
- Before a substantial change, state the intended behaviour briefly. After the change, build and run the relevant tests and report what passed, failed or remains untested.
- Prefer simple, explicit DSP and data flow over speculative abstractions or unnecessary dependencies.
- Do not add non-functional placeholders or claim a future stage is implemented when only its UI exists.
- Document algorithm assumptions, units, bounds, smoothing, latency and failure modes.
- Avoid busy-waiting, audio-thread locks and dynamic allocation in the audio callback.
- If modifying a file, provide the complete modified file when responding to the user, not only a patch fragment.
- Do not invent requirements that remove original product features. If a feature is too difficult for the MVP, retain it in the roadmap and mark it as deferred, with a concrete implementation/test plan.
- Keep `AGENTS.md`, README, build scripts and actual code consistent.

## 13. Definition of product success

The product succeeds when it can analyse an external master and a DAW Source, display meaningful octave-by-octave differences in a Youlean-inspired dashboard, propose and apply coordinated corrections, and produce a controlled result that approaches the Reference without blindly forcing every metric to match.

The initial useful product must include reference loading, octave-first analysis, a separate Matching Engine, EQ matching, low-end mono/M-S management, True Peak output control, Match Intensity, level-matched A/B and Delta monitoring. Advanced requirements—percussive-prominence transient shaping, Smart De-Resonator, global and multiband drive/saturation, glue compression, BPM-aware release, multiband density matching and advanced stereo matching—must remain explicit implementation goals even if delivered in later stages.

The central engineering challenge is not merely implementing many processors. It is deciding **when to intervene, which feature to correct, which processor to use, how strongly to apply it, and when to stop**.
