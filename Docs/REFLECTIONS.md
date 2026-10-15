# Reflections

A short log, one entry per stage, written only after that stage's tests pass. Honest
assessment of what was built, what is weak, and what to watch. Not a changelog.

## Stage 8 — resonance, drive and stereo refinement

Built: `ResonanceDetector` (local-median narrow-peak detection), `Saturator`,
`StereoWidth`, `DynamicEq`, plus a shared RBJ band-pass.

- **What worked:** driving each unit from a synthetic, deterministic input kept the tests
  crisp (flat spectrum → no resonance; loud band → cut; out-of-band → untouched).
- **What is weak:** detection and dynamic-EQ thresholds are unvalidated guesses. `ResonanceDetector`
  was only tested on hand-made spectra, never on real music, so its false-positive rate (the
  exact risk the spec warns about — synths and basses are legitimately resonant) is unknown.
- **Design choice worth noting:** `DynamicEq` is a band-split, subtractive design, not a
  time-varying biquad. It is stable and cheap but the colour it imposes may differ from a
  true dynamic EQ; revisit in hardening.
- **Risk:** none of this has ever been heard. Everything rests on synthetic signals.

## Stage 9 — full-chain coordination

Built: the full ordered chain (each module bypassable), `ChainCoordinator` (processing
budget / penalty on excess), `ChainReporter` (post-render distance), and the first
integration tests.

- **What worked:** wiring every standalone module into one `process()` exposed how much was
  previously unconnected — `planDynamics` and `detectResonances` were dead code until now.
- **What the tests cover:** identical Source/Reference stays close; an unreachable target is
  reported; the full chain stays finite and under the ceiling; silence is safe. They passed
  first time, which is mild evidence of good modular boundaries — but they are still
  synthetic.
- **Known gaps carried forward:** compressor / transient shaper / multiband are **per-channel
  with no stereo link**, so they can shift the image; the limiter controls **sample peak, not
  True Peak**; the chain order is a reasonable guess (the spec allows reordering if documented).
- **Process observation:** editing the flat source list in `src/CMakeLists.txt` by hand has
  caused repeat friction (a duplicate entry and a broken root file slipped in once). Worth
  revisiting.
- **Next:** Stage 10 hardening — enforce True Peak, add an offline render tool so the pipeline
  is usable without a DAW, document DSP definitions, and run the validation matrix at greater
  breadth. The JUCE shell remains the single biggest untested unknown.

## Stage 10 — hardening, offline tool, docs

Built: `TruePeakGuard` (static scale-down to a dBTP ceiling), `WavFile` (16-bit/float read,
16-bit write), `masterizer_cli` (reference + source WAV -> analyse -> process -> write), and
`Docs/ARCHITECTURE.md`, `Docs/DSP_DEFINITIONS.md`, `Docs/TESTING.md`.

- **What worked:** the CLI is the first genuinely usable artifact — the pipeline now runs
  end-to-end on real files (smoke test: 3 proposals, target shortfall reported, True Peak
  verified). It also forced the integration points to be honest.
- **What is weak / unresolved:** the limiter still controls **sample peak**; `TruePeakGuard`
  mitigates True Peak with a single static gain (not a look-ahead limiter). Dynamics modules
  remain **per-channel (no stereo link)**. Thresholds and heuristics are uncalibrated.
- **Biggest risk, unchanged:** *nothing has been validated on real music or in a listening
  test, and there is no plugin.* Every number rests on synthetic signals. The core is
  coherent and well-tested; its musical value is entirely unproven.
- **Recommended next:** the JUCE VST3 shell — make it audible, validate the real-time path,
  and only then calibrate thresholds against actual tracks. Do not add more DSP before that.

## Phase 1 (UI) — parameter model + dashboard presenter

Built: `domain/state/Parameters` (stable ids, ranges, defaults, clamping) and
`application/DashboardModel` (pure presenter: per-band source/reference/difference/correction,
LUFS, True Peak, reference state, A/B and Delta flags, warning text).

- **Why this first:** the risky part of a UI is the *logic behind it*, not the pixels. Moving
  all of that into tested, JUCE-free code means the eventual editor is a thin render layer.
- **Reliability check:** these are ordinary TDD functions; they are as reliable as any other
  part of the core. The uncovered risk is now only "does the drawing and interaction feel
  right to a human", which genuinely needs a person at a screen in a DAW.
- **Still missing:** the JUCE adapter itself (no code exists that opens a window or runs an
  audio callback), so there is nothing visual to test yet. `Docs/WINDOWS_TESTING.md` covers
  building and testing the current core + CLI on Windows without an agent.


