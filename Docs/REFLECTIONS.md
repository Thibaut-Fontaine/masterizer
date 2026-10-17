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

## Speed S0+S1+S3 — profiling, True Peak table, loudness-only loop

- **What changed:** added `--timing` (per-phase ms); `truePeak()` now uses a precomputed
  polyphase kernel table instead of `sin`/`cos` per tap per sample; the loudness targeting loop
  measures LUFS only and computes True Peak once at the end (it previously recomputed the
  expensive True Peak on every iteration via `analyzeLoudness`).
- **Measured:** 18 min 11 s → **1 min 57 s (9.3×)** on the 4.5-min track. Same result:
  −10.61 LUFS, −1.00 dBTP, +9.45 dB.
- **Regression contract:** held (LUFS/TP/gain identical to OURS2).
- **Surprise:** the hotspot moved. `analyzeSource` is now **99.5 s of 117 s (85%)**; read 1.7 s,
  process 1.3 s, target 12.5 s, validate 1.3 s, write 1.2 s.
- **Next:** parallelize / single-pass the analysis. That is the only thing standing between us
  and the ≤30 s target.

## Speed S4+S3b+S6 — parallel analysis, fewer measurements, hop 1024

- **What changed:** source and reference analysis run concurrently; the six independent feature
  passes (octave, spectrum, variation, transient, resonance, stereo) run concurrently inside
  each analysis; the targeting loop reuses its best measurement instead of re-measuring, and the
  CLI builds the output report from the targeting result instead of calling `validateOutput`
  again; analysis hop 512 → 1024.
- **Measured:** 1 min 57 s → **27.7 s** (total 39× from the 18 min baseline). Breakdown:
  read 1.7 s, analyze 13.1 s, process 1.4 s, target 10.7 s, write 0.8 s.
- **Regression contract:** held exactly — OURS−MASTER metrics identical to the pre-optimization
  render (LUFS −0.10, TP −1.12, crest −1.17, per-band values unchanged).
- **Surprise:** the True Peak table was worth ~9× on its own; the remaining cost is now split
  between analysis (13 s) and the loudness search (10.7 s). Further gains would need single-pass
  analysis or a cheaper LUFS search, but the 30 s stop rule is met, so we stop here.

## Backlog steps 0–8 — diagnostic to limiter

- **0. Diagnostic.** Mids/highs carry the human change; the master compresses HMid/Presence more
  and lows less; we were ~1.2 dB globally flatter; saturation evidence weak. Report:
  `Docs/DIAGNOSTIC_DAXJ.md`.
- **1. Crest guard.** Stops before global crest falls 0.5 dB below the reference; reports the
  shortfall. `--no-crest-guard` restores exact-LUFS behaviour.
- **2. Edge shelves.** Sub and VHigh now use a low/high shelf, so the air gap can be addressed.
- **3. Tape HF.** Reduced to −0.1 dB at 14 kHz, mix 0.5.
- **4. Fine EQ.** Third-octave design from the fine spectra, bounded and capped, opt-in via
  `--fine-eq` until listening validates it.
- **5. Upper-mid dynamic EQ.** `planDynamics` enables a gentle 3 kHz band when our HMid/Presence
  variation exceeds the reference.
- **6. Per-band correlation.** Real L/R cross-spectrum correlation replaces the misleading
  `(M−S)/(M+S)` estimate.
- **7. Near-mono guard.** Width matching is disabled when the reference width < 0.05.
- **8. Look-ahead limiter.** Anticipatory sliding-max gain, wired into loudness targeting.
- **Effect on the test track (OURS7):** LUFS −11.34 (shortfall 0.84 vs −10.50), TP −1.00,
  crest 10.75 vs master 10.81 (**−0.06 dB**, was −1.17), prominence −0.01. The trade-off is now
  explicit: dynamics match, loudness is 0.84 LU short; `--no-crest-guard` recovers the LUFS but
  flattens the track.

## HF follow-up — the 8-12 kHz gap

The user heard the highs as stronger on the master. Third-octave analysis put the gap at
**7.9-10 kHz (+1.5…+1.9 dB)**, not >12 kHz. The air shelf was at 12 kHz (under-correcting 8-12 k)
and the High band was rejected by the confidence gate. Moving the shelf to **8 kHz** closed it
(7.9 k +0.41, 10 k −0.28, High +0.16). With `--ceiling -0.1 dBTP`, LUFS −10.54 vs −10.50 and
crest 10.58 vs 10.74. Lesson: measure *where* in the spectrum a complaint lives before changing
the design; octave averages hid a narrow 8-12 kHz gap.




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

## Final reflection before Windows (whole project)

State at this point: **53 commits**, **211 passing cases**, 225 files / 372 functions,
~7,700 lines, 7 docs. Stages 0-10 (core) + Phase 1 UI logic + an offline CLI.

**What is genuinely proven**
- The pure logic is well covered: DSP, analysis, matching, chain and presenter each have
  focused unit tests, plus a handful of integration tests. Everything is deterministic.
- The two measurements we can check against a real standard are correct: the K-weighting
  coefficients reproduce BS.1770-4 exactly, and the 997 Hz calibration lands on -3.0103 /
  -23.0103 LUFS. True Peak over-reads nothing and never under-reads the sample peak.
- The pipeline runs end-to-end on real files (`masterizer_cli`) and the size rules are
  enforced, so the codebase is uniform.

**What is NOT proven — and matters most**
- **Musical quality is unproven.** Every test uses synthetic signals. The product's whole
  value is "does the result sound closer to the reference", and nothing has been heard. Worse,
  the spec explicitly says a lower spectral distance is *not* evidence of better sound — so the
  `ChainReporter` number must not be trusted as a success metric.
- **Thresholds and heuristics are guesses.** De-resonator peak ratio, dynamic-EQ threshold,
  saturation drive, glue/multiband planning, transient weighting — all initial values. The
  spec's own warning (synths and basses are legitimately resonant) means the de-resonator is
  the most likely to misfire on real music.
- **Structural gaps:** dynamics modules are per-channel (no stereo link → possible image
  shift); the limiter controls sample peak and `TruePeakGuard` is a static scale-down, not a
  look-ahead limiter; loudness targets can be missed.
- **No host, no UI, no real-time path.** No plugin exists, so latency, automation, state
  restore, block sizes and the audio callback are entirely untested. Nothing visual exists.
- **The tests are self-authored.** The only independent oracles are the standards above. The
  compressor episode (a test that measured the wrong thing and passed for the wrong reason)
  is a reminder that synthetic fixtures encode my own assumptions.

**Process wins**
- TDD caught real defects rather than cosmetic ones: True Peak edge ringing, the mono-bass
  filter being the wrong order (and its "strict" mode making it *less* mono), the compressor
  test measuring the global peak instead of steady state, a broken root `CMakeLists.txt`, and
  false positives in the size checker.

**Process cost worth naming**
- The "≤5 functions per file" rule produced 225 files for ~7,700 lines and constant churn in
  the one flat CMake source list. It kept every unit small and testable, but it trades
  navigability and build-file maintenance for granularity. That is a deliberate trade, not a
  free win.

**What the Windows trip can and cannot prove**
- Can: MSVC portability, that the suite passes on Windows, and that the CLI renders real
  tracks without crashing.
- Cannot: sound quality, calibration, the UI, or host behaviour. Those need a plugin and ears.

**Honest recommendation**
1. Ship the JUCE shell next so the engine becomes audible, then **listen before tuning**.
2. Treat every threshold as a hypothesis; expect to change most of them once real music runs
   through it.
3. Stereo-link the dynamics and replace the static True Peak guard with a look-ahead limiter.
4. Do not present "distance reduced" as success.

**Uncertainty statement:** the code is coherent, portable and honestly bounded, but I cannot
vouch for how it sounds. That judgement is entirely still ahead.

## Pre-VST step 1 — surface the Mix Advisor

- **What changed:** `adviseFeedback()` now returns a `MixFeedback` stamped with the ruleset
  version; `SourceNotes` renders hedged one-line notes; the CLI prints them after analysis; the
  dashboard presenter carries the suggestion list. No DSP, chain, targeting or render code was
  touched.
- **Verified:** 249 cases and all 4 CTest suites pass. The real-pair render is unchanged in its
  metrics: −11.53 LUFS, −1.00 dBTP, crest 10.67.
- **What is weak:** the advisor stays silent on the Dax J pair — expected for a close match and
  consistent with "stay silent when evidence is weak", but it means the feature has not yet been
  heard against a real problem mix. Only 5 of 11 rules have dedicated fixtures; action text is
  still hard-coded in the rules; there is no cap/de-duplication pass.
- **Next:** fix the CLI numeric-flag leak (without changing `--gain` semantics), then extend the
  parameter model and dashboard for the editor. `ChainCoordinator` remains deliberately unwired:
  applying its budget would change the approved master.

## Pre-VST step 2 — CLI numeric-flag leak

- **What changed:** `--intensity` and `--ceiling` now parse into a local value instead of writing
  through `outputGainDb`. `--gain` still writes `outputGainDb` exactly as before, so the
  targeting path is untouched.
- **TDD evidence:** the new regression test failed on `options.outputGainDb` before the fix, then
  passed. 252 cases and all 4 suites are green.
- **Verified on the real pair:** `--intensity 1 --ceiling -1` renders the same result as the
  default path (−11.53 LUFS, −1.00 dBTP, crest 10.67), confirming the change only removes the
  accidental extra gain.
- **What is weak:** `--gain` is still a starting gain for the targeter, not a final output trim.
  Changing that would alter the approved master, so it stays documented as a future decision,
  not a silent behaviour change.
- **Next:** extend the domain parameter model for the VST (profile, fine EQ, tape, crest guard),
  then the dashboard presenter.

## Pre-VST step 3 — VST-facing parameter ids

- **What changed:** `ParameterId` now includes `MasterProfile` (0 = soft, 1 = loud),
  `FineEqEnabled`, `TapeEnabled`, `LimiterEnabled` and `CrestGuardEnabled`, with stable string
  ids and ranges. Defaults follow the approved CLI sound: tape on, fine EQ off, limiter and crest
  guard on.
- **Verified:** 255 cases and all 4 suites pass. No DSP, chain, targeting or render code changed.
- **What is weak:** the enum is now the plugin's parameter vocabulary, but there is still no
  mapping from parameters to `ChainSettings` — that belongs to the plugin adapter. The soft/loud
  value is deliberately a 0/1 parameter; the adapter maps it to `MasterProfile`.
- **Next:** extend `DashboardModel` with the fields the editor needs (profile, per-module
  activity, proposal confidence, suggestions).

## Pre-VST step 4 — dashboard presenter for the editor

- **What changed:** `BandView` now exposes the proposal confidence; `ModuleView`
  (`id`, `enabled`, `amount`) and the active `MasterProfile` flow through the presenter. The
  editor can render corrections, confidence and module state without touching DSP internals.
- **Verified:** 257 cases and all 4 suites pass. The presenter remains pure and deterministic;
  the CLI render is untouched.
- **What is weak:** module activity is caller-supplied, not derived from `ChainSettings` yet.
  That mapping belongs to the plugin adapter, where the chain is assembled. There is still no
  drawing code, so "does it feel right" remains untested until the JUCE editor exists.
- **Next:** the stateful real-time processors, starting with the EQ and mono/M-S stages. This is
  the largest remaining pre-VST item.



