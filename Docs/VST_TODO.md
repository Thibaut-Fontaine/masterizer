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

## Milestones

### M0 — Build plumbing

- [ ] Add JUCE **8.0.15** pinned. Prefer `FetchContent` with `GIT_TAG 8.0.15`;
      fall back to a submodule if the Windows/CI network is a problem.
- [ ] Add `MASTERIZER_BUILD_PLUGIN` (default OFF for core CI, ON for the plugin presets).
- [ ] Add `juce_add_plugin(Masterizer ...)`:
      `FORMATS VST3`, `IS_SYNTH FALSE`, `NEEDS_MIDI_INPUT FALSE`,
      `NEEDS_MIDI_OUTPUT FALSE`, `COPY_PLUGIN_AFTER_BUILD TRUE`,
      `VST3_CATEGORIES Fx`, stereo in/out.
- [ ] Link `masterizer_core`; no DSP in `src/plugin`.
- [ ] Add a plugin CI job (Ubuntu + Windows) with the Linux dev packages and artifact
      upload. Keep the core `unit`/`style_rules`/`includes`/`portability` jobs.
- [ ] Local sandbox fallback: extract the missing dev `.deb`s into a local sysroot
      (`apt-get download` + `dpkg-deb -x`) so the VST3 can be compiled here.
- **Acceptance:** plugin target configures, builds and links; core tests still pass
  with the plugin OFF; CI is green.
- **Tests:** build only. No audio yet.

### M1 — Minimal pass-through VST3

- [ ] `PluginProcessor`: stereo bus, `processBlock` copies input to output,
      bypass-safe, `getTailLengthSeconds` 0, no MIDI.
- [ ] `createEditor` returns `juce::GenericAudioProcessorEditor` for now.
- [ ] Report zero latency (nothing delayed yet).
- [ ] Add a plugin smoke test (headless if possible): instantiate, prepare at 48 kHz
      and 512 frames, process silence and a sine, assert finite output, check bypass.
- [ ] Run `pluginval` at a basic strictness.
- **Acceptance:** VST3 loads in a DAW, passes audio, bypass works, no clicks, no
  crashes on prepare/release.
- **Commit:** `feat(plugin): minimal pass-through VST3`.

### M2 — Parameter layout and state

- [ ] Build the `AudioProcessorValueTreeState` layout from
      `domain/state/Parameters` (master enable, `MasterProfile` 0=soft/1=loud,
      intensity, ceiling, output gain, limiter, EQ, fine EQ, mono, saturation, tape,
      stereo, transient, glue, multiband, dynamic EQ, resonance, crest guard).
- [ ] Use stable parameter ids; ranges/defaults from `specOf`.
- [ ] `getStateInformation` / `setStateInformation` round-trip.
- [ ] Automation-safe reads; no parameter changes that allocate on the audio thread.
- **Acceptance:** parameter list is complete; defaults match the CLI sound; state
  restore works in a fresh instance; automation does not glitch.
- **Tests:** headless parameter count/ids/ranges/defaults and state round-trip.
- **Commit:** `feat(plugin): parameter layout and state`.

### M3 — Reference workflow

- [ ] `JuceAudioDecoder` implementing `IAudioDecoder` with JUCE
      `AudioFormatManager` (WAV/FLAC/MP3/AIFF as available).
- [ ] `ReferenceController`: file chooser + drag-and-drop, `ReferenceKey`
      (path + size + mtime), `ReferenceManager`, `ProfileCache`.
- [ ] Background decode → `analyzeSource` → cache profile → **drop audio**.
- [ ] Status: unloaded / loading / ready / stale / failed; missing/changed file safe.
- [ ] Simple UI: "Load reference" button, name, status label.
- **Acceptance:** a real reference loads and analyses without blocking the audio
  thread; an unsupported/missing file shows an error and does not crash.
- **Tests:** fake `IAudioDecoder` for controller logic; real-file smoke by hand.
- **Commit:** `feat(plugin): reference loading and analysis worker`.

### M4 — Source capture and analysis

- [ ] Preallocated capture FIFO/ring (no allocation in `processBlock`).
- [ ] "Analyze" starts a capture session; default 60 s, hard cap documented.
- [ ] Worker drains the capture and runs `analyzeSource`.
- [ ] Build `MixFeedback` (Source notes) from the two profiles.
- [ ] Status/progress in the UI; silence/near-silence/mono/anti-phase safe.
- **Acceptance:** capture a real section, get a ready Source profile, then a plan.
- **Tests:** FIFO/controller with synthetic blocks; silence and anti-phase.
- **Commit:** `feat(plugin): source capture and analysis`.

### M5 — Plan construction and handoff

- [ ] Add a JUCE-free `application/PlanBuilder` that turns the two profiles +
      `ChainSettings` into a `RealtimePlan`:
      - `MatchingEngine::propose` → `designEq` (Match Intensity via `scaleEqAmount`),
      - `planDynamics`,
      - `detectResonances` bands,
      - `designFineEq` bands,
      - `preLimiterGainDb` from source/reference LUFS + crest guard,
      - `MasterProfile` / ceiling,
      - `MixFeedback`.
- [ ] Publish an immutable `shared_ptr<const RealtimePlan>`; audio thread only swaps.
- [ ] Build `RealtimeChain` off-thread; apply on stop or short crossfade.
- [ ] `processBlock` runs the chain; `setLatencySamples` from the limiter.
- [ ] Rebuild the plan when the reference, source profile, profile, intensity or a
      module enable changes; never in the audio callback.
- **Acceptance:** processed audio, no clicks at plan swap, block-size independent,
  latency reported, bypass restores dry audio.
- **Tests:** `PlanBuilder` unit tests (JUCE-free); audio-thread smoke with silence
  and a sine; plan-swap test.
- **Commit:** `feat(plugin): plan construction and real-time handoff`.

### M6 — Simple dashboard UI

Replace `GenericAudioProcessorEditor` with one page. Plain JUCE controls only.

```text
+----------------------------------------------------------------------+
| Reference: [Load...]  name.wav  [Ready]     Profile [Soft v]         |
| Intensity [----o----]  Analyze [ Capture ]  Apply [ Apply ]  [Bypass]|
+----------------------------------------------------------------------+
| Source vs Reference — 10 octave bands                                |
|   Sub   Low   ULow  LowMid  Mid  UMid  HMid  Presence  High  VHigh   |
|   ███   ███   ███   ███    ███  ███   ███   ███       ███   ███     |
|   ░░░   ░░░   ░░░   ░░░    ░░░  ░░░   ░░░   ░░░       ░░░   ░░░     |
|   +2.1  -0.8  ...                                                    |
+----------------------------------------------------------------------+
| LUFS -11.5   TP -1.0 dBTP   Shortfall 1.0 LU   A/B [A][B] [Delta]   |
+----------------------------------------------------------------------+
| Modules: [EQ] [Fine EQ] [Mono] [Limiter] [Tape] [Crest] [Glue] ...   |
+----------------------------------------------------------------------+
| Source notes (read-only):                                             |
|  - R1 [high] ... (verify by ear)                                      |
+----------------------------------------------------------------------+
```

- [ ] `HeaderComponent`: reference load/status, profile combo, intensity slider,
      Analyze/Apply, bypass.
- [ ] `BandMeterComponent`: 10 bands; Source bar, Reference bar, difference text,
      correction amount/confidence. Simple `paint()` rectangles, no animation.
- [ ] `MeterComponent`: LUFS, True Peak, shortfall, warning color.
- [ ] `MonitorComponent`: A/B buttons, Delta toggle, monitoring-gain indicator.
- [ ] `ModuleTogglesComponent`: compact wrapped row of toggles. Planner-driven
      modules show their state but remain user-overridable.
- [ ] `SourceNotesComponent`: read-only multiline text; collapsible; empty state
      "No source notes."; never an unhedged instrument claim.
- [ ] All values come from `DashboardModel`; the editor never calls DSP directly.
- [ ] Resizable 900×600 default; readable at 100% scaling.
- **Acceptance:** a human can load a reference, capture, apply, A/B, bypass and read
  the notes without a manual. No drawing work on the audio thread.
- **Tests:** presenter already tested; manual UI checklist. Add an editor smoke test
  if it can run headless.
- **Commit:** `feat(plugin): simple dashboard editor`.

### M7 — A/B, Delta and monitoring gain

- [ ] Delay the dry path by `latencySamples()` so A/B is aligned.
- [ ] A = dry, B = processed; fast switching without clicks.
- [ ] Delta = processed (delay-compensated) − dry.
- [ ] Monitoring gain = source LUFS − processed LUFS; separate from export gain.
- [ ] Show "Delta reveals changes, not whether they sound better."
- **Acceptance:** A/B is level-matched and aligned; Delta contains only the change;
  monitoring gain never changes the exported level.
- **Commit:** `feat(plugin): A/B, Delta and monitoring gain`.

### M8 — True Peak and limiter hardening

- [ ] Decide: oversampled limiter detector **or** a final True Peak safety stage.
- [ ] Verify at 44.1/48/88.2/96 kHz; the current limiter is sample-peak only.
- [ ] Flush or document the limiter tail at the end of a stream.
- [ ] Confirm `setLatencySamples` and A/B/Delta compensation after any change.
- **Acceptance:** output is at/below the selected ceiling (soft −1.0, loud −0.1 dBTP)
  by a documented method, and the UI labels it correctly.
- **Commit:** `feat(plugin): true-peak-safe output stage`.

### M9 — Hardening and host validation

- [ ] Block sizes 32–2048; sample rates 44.1/48/88.2/96 kHz.
- [ ] Mono, wide, anti-phase, silence, near-silence, very quiet/loud inputs.
- [ ] Bypass, automation, project save/restore, offline render.
- [ ] Missing/changed reference, unsupported file.
- [ ] NaN/inf sanitisation; no denormal stalls; CPU/memory check.
- [ ] `pluginval` strictness 5+ on Windows; at least one real DAW session.
- [ ] Record results in `Docs/`; no claim without a run.
- **Commit:** `test(plugin): host validation matrix`.

### M10 — Release packaging

- [ ] Windows installer or documented copy-to-VST3-folder build.
- [ ] Versioning, release notes, reproducible build instructions.
- [ ] JUCE/VST3 licence review.
- [ ] CI artifacts for Windows and Linux.
- [ ] Update `README`, `AGENTS.md`, `Docs/WINDOWS_TESTING.md`.
- **Commit:** `chore: VST3 release packaging`.

## Simple UI rules

- One page, no tabs. Dark neutral background, two accent colors, system font.
- Use stock `juce::Label`, `Slider`, `ComboBox`, `TextButton`, `ToggleButton`,
  `TextEditor`.
- No custom look-and-feel, no images, no animation, no OpenGL.
- The octave view is the only custom drawing: 10 groups of two bars plus text.
- Keep every component in its own file to satisfy the ≤5-functions rule.
- Show numbers with sensible precision: LUFS 0.1, dB 0.1, intensity 0–200%.
- Warnings are plain text, not modal dialogs (except load failures).
- Never block the message thread on analysis; show "Analysing…" and keep the editor
  responsive.

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
  PluginState.h/.cpp
  ui/HeaderComponent.h/.cpp
  ui/BandMeterComponent.h/.cpp
  ui/MeterComponent.h/.cpp
  ui/ModuleTogglesComponent.h/.cpp
  ui/SourceNotesComponent.h/.cpp
  ui/MonitorComponent.h/.cpp
```

JUCE-free additions:

```text
application/PlanBuilder.h/.cpp      # profiles -> RealtimePlan
application/RealtimePlan.h          # immutable plan contract
```

## Testing strategy

- **Core/application:** normal in-repo tests, no JUCE, no host.
- **Plugin headless tests:** parameter layout, state round-trip, `prepareToPlay` /
  `processBlock` with silence/sine, bypass, plan swap. Instantiate the processor
  without an editor or audio device.
- **Editor:** manual checklist; add a headless smoke test only if it is reliable.
- **`pluginval`:** CI on Windows and Linux if the runner supports it.
- **Real DAW:** Windows, at least one host; load, play, seek, save/restore, offline
  render, bypass.
- **Listening:** level-matched A/B across a few subgenres; never use "lower spectral
  distance" as proof.

## Risks / blockers

- **Sandbox cannot build JUCE yet.** No passwordless sudo; Docker socket denied.
  Plan: local deb sysroot for compile/link, CI for the real build. Verify early.
- **Editor validation needs a human.** Automated tests cannot judge layout/feel.
- **Threading bugs** are the main correctness risk: keep one analysis worker, one
  immutable plan, no locks in `processBlock`.
- **True Peak** is the main quality risk: the current real-time limiter is
  sample-peak only.
- **Capture UX** is unusual for a VST; make the 60 s default and the cap explicit.
- **`≤5 functions` rule** with JUCE components: split early, do not grow one editor
  file.

## Definition of done (Gate 2)

- VST3 loads in a real Windows DAW and survives play/seek/save/restore/offline render.
- Reference loads and analyses off-thread; missing file is handled.
- Source capture → plan → real-time processing works on a real track.
- Bypass, A/B, Delta, soft/loud, Match Intensity and module toggles work.
- Source Notes are visible and never affect audio.
- True Peak ceiling is honestly enforced and labelled.
- Latency is reported and compensated.
- `pluginval` passes; the validation matrix is recorded.
- The simple UI is usable without a manual.

## First step

**M0 + M1 only:** add the pinned JUCE build plumbing and a pass-through stereo VST3
with `GenericAudioProcessorEditor`. Verify compile/link in the sandbox (or CI),
load it in a DAW, then stop and reflect before the custom UI.
