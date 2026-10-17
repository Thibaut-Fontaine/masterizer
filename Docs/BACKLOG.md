# Backlog

Prioritized work list. Each item has a definition of done. Update this file when an item
lands. Context at the start of this backlog: the engine matched a human master within ~0.4 dB on
most bands and matched crest/prominence, but was **too flat** (over-limited to hit exact LUFS at
−1 dBTP) and **lacked air** above ~11.5 kHz.

## Status (2026-10-17)

Done: **0** diagnostic (`Docs/DIAGNOSTIC_DAXJ.md`), **1** crest guard + `--no-crest-guard`,
**2** edge-band shelves, **3** tape HF reduced, **4** third-octave fine EQ (opt-in `--fine-eq`),
**5** conditional upper-mid dynamic EQ, **6** true per-band correlation, **7** near-mono guard,
**8** look-ahead limiter. Performance S0–S6 done (~27–32 s per track). Also done: dual delivery
profiles (`--profile soft|loud|both`), **278 green tests**, and the pre-VST foundation
(Mix Advisor Source Notes, CLI parsing fix, VST parameter ids, dashboard fields, stateful block
processors, `RealtimeChain`, real-time handoff plan).
Next: the **VST shell (Gate 2)**, then Match Intensity coverage and the validation matrix
(Gate 3). See `Docs/PROJECT_BRIEF.md` and `Docs/REALTIME_PLAN.md`.

## Next (after steps 0-8)

The pre-VST foundation is delivered:

- Mix Advisor wired as read-only Source Notes (CLI + dashboard presenter).
- CLI numeric-flag leak fixed (`--intensity`/`--ceiling` no longer change `outputGainDb`).
- VST-facing parameter ids (`MasterProfile`, fine EQ, tape, limiter, crest guard).
- Dashboard fields for profile, confidence and module activity.
- Stateful block processors + `RealtimeChain` (block-size independent, latency reported).
- Real-time threading/capture/handoff design in `Docs/REALTIME_PLAN.md`.

Remaining, in order:

1. **Gate 2 — JUCE VST3 shell**, in small buildable steps: minimal pass-through plugin ->
   decoder/reference workflow -> background analysis -> plan handoff -> simple dashboard ->
   Source Notes. The ordered checklist, UI sketch and acceptance criteria live in
   `Docs/VST_TODO.md`. Expose `MasterProfile` (soft/loud), fine EQ, tape and crest guard as
   controls.
2. **Extend Match Intensity** beyond EQ to the other automatic modules, with module-specific
   curves. This changes the sound, so it needs a listening decision.
3. **Gate 3 — validation and release:** breadth matrix, pluginval, a real DAW, packaging,
   licences, release notes.
4. **Section analysis** (roadmap Step 9) remains deferred with a concrete test plan.

`ChainCoordinator` and a true final output trim are deliberately **not** wired: both would change
the approved master. Treat them as sound decisions, not refactors.

## Metric roles (do not drift from this)

- True Peak: **hard constraint** (≤ −1.0 dBTP). Never match a hotter reference.
- LUFS: **soft target**. Approach it, but stop at the dynamics guard; report the shortfall.
- Crest / approx LRA: **guards**, not targets. If they fall too far below the reference, back off.
- Width: **optional/diagnostic**. Off for near-mono references; never chase per-band ratios.
- Lower spectral distance ≠ better sound.

- **Delivery profiles:** `--profile soft|loud|both`; soft = −1.0 dBTP, loud = −0.1 dBTP. Both
  use the same chain and crest guard; only the ceiling differs. Exposed to the VST as one
  `MasterProfile` parameter.

## Delivered backlog steps 0-8, and standing rules

> Steps 0-8 and performance S0-S6 are delivered. Item 10 (VST) is tracked in "Next"; items
> 11-13 are standing rules. This section is kept so the original definitions of done remain
> auditable.

0. **Characterise the residual difference first (diagnostic; no DSP changes).**
   The listening notes ("lead slightly louder in mids/highs", "master sounds a bit
   saturated/harsher", "not very clear") could be saturation/exciter, static EQ air, dynamic
   upper-mid control, or limiter attack behaviour. Measure before choosing.
   - **Section-wise analysis:** split the track into ~8–16 s windows (or detected sections) and
     compute per-band relative level, width, variation and transient density for PRE, MASTER and
     OURS. Find the sections where the lead is exposed; check whether the band difference flips
     by section (→ dynamic EQ/multiband) or is constant (→ static EQ).
   - **Harmonic analysis:** pick 2–3 sustained lead notes; compare the spectrum *between* the
     fundamental's partials in PRE vs MASTER at matched gain; estimate inter-harmonic energy/THD.
     Extra inter-harmonic energy → saturation/exciter; a smooth tilt → EQ.
   - **Attack-envelope comparison:** detect lead/kick onsets and compare attack slope/envelope.
     Sharper master attacks → limiter behaviour, not harmonics.
   - **Level-matched delta:** render MASTER − PREMASTER at equal loudness; classify the residual
     as fizz/edge (harmonics), smooth tilt (EQ), transient reshaping (limiter), or movement
     (dynamics).
   - **Output:** a short written diagnosis (numbers + ranked likely causes + confidence) and a
     decision on which of items 1–8 to prioritise. Save it under `Docs/`.
   *Done when:* the diagnostic report exists and the next item is chosen from evidence, not
   impression.

1. **Dynamics guard in loudness targeting.**
   Stop the closed loop when crest is more than ~0.5 dB below the reference (or approx LRA
   ~0.5 LU tighter); accept a small shortfall. Addresses "ours sounds flatter than the master".
   *Done when:* on the Dax J track, OURS−MASTER crest ≥ −0.5 dB and the shortfall is reported.
2. **Edge-band shelves in the EQ.**
   `EqDesigner` skips Sub and VHigh because they have no geometric centre. Use a high shelf for
   VHigh and a low shelf for Sub.
   *Done when:* |OURS−MASTER| at VHigh < 0.5 dB on the test track.
3. **Reduce/remove the tape HF roll-off.**
   The tape's −0.4 dB shelf at 14 kHz fights the air. Lower it (or the tape mix) while keeping
   the stage subtle and toggleable.
   *Done when:* the tape no longer measurably reduces VHigh, and A/B still sounds like tape.
4. **Finer EQ resolution.**
   Replace 10 octave bands with finer log-spaced bins (e.g. third-octave) and a capped filter
   count to shape 1.4–5.8 kHz more accurately (Presence is +0.43 dB vs master).
   *Done when:* Presence |OURS−MASTER| < 0.3 dB with ≤ N filters.
5. **Dynamic EQ / multiband in 1.4–5.8 kHz — conditional.**
   Only if listening confirms the master controls that region dynamically (its HMid variation
   dropped the most). Do not add on spectral evidence alone.
6. **Fix the per-band stereo metric.**
   Replace the `(M−S)/(M+S)` energy estimate with real per-band L/R correlation; the current
   metric can read "wider" while the ear hears "narrower".
   *Done when:* the metric matches a straightforward time-domain correlation on band-limited
   signals.
7. **Near-mono stereo guard.**
   Never enable per-band width matching when the reference width is tiny (e.g. < 0.05).
   *Done when:* the stereo stage is a no-op on the Dax J track.
8. **Look-ahead True Peak limiter.**
   The current limiter is zero-look-ahead; it did not sound crushed, but a look-ahead design
   with a gain-reduction cap is the right quality bar. Revisit after 1–4.
9. **Performance** — see the dedicated section below. Prerequisite for the VST.
10. **VST shell** — only once 1–4 and the sound itself are accepted.
11. **Never** match a hot reference (+0.12 dBTP) or force exact LUFS past the dynamics guard.
12. **Never** optimize crest/LRA/width as numbers; listen at matched level.
13. **Keep the metric-role strategy in the docs** (this section) and `AGENTS.md` so the project
    does not drift back to "match every metric".

## Rendering speed (current: ~18 min for a 4.5-min track)

Target: **≤ 30 s** cold per track (stretch ≤ 10 s warm), with numerical results unchanged
within tolerance. Hardware-dependent; record the actual machine in the speed log.

### Operating rules

- **Measure before optimizing.** No change without a baseline and a measured delta.
- **One optimization per commit.** Never mix S-steps in a single commit.
- **Benchmark protocol:** Release build, same input track, same machine/load, **3 runs, take the
  median** wall-clock. Report the phase breakdown from `--timing`, not just total time.
- **Regression contract.** Golden metrics from the test track must match after every step:
  integrated LUFS ±0.05, True Peak ±0.05 dB, crest ±0.1 dB, per-band relative level ±0.1 dB,
  width ±0.005, per-band correlation ±0.01. Outside that = a behaviour change, not an
  optimization → stop and investigate before continuing.
- **Stop rule.** Once ≤ 30 s cold is reached, stop; do not micro-optimize further.
- **Checkpoint after every step:**
  1. `ctest --preset linux-release` (all suites green),
  2. run the benchmark 3× and record the median,
  3. update the speed log below,
  4. `git commit` (one step, one commit) + push,
  5. append a short **self-reflection** to `Docs/REFLECTIONS.md` (template below).

### Reflection template (per step)

> What changed · measured speedup (before → after) · did the regression contract hold? ·
> what surprised us / what is still a hotspot · next step.

### S0 — Baseline + measurement harness (no optimization)

- Add `--timing` to the CLI: per-phase ms for read, analyze source, analyze reference, process,
  loudness targeting, validate, write.
- Save golden outputs for the reference track (`OURS2.wav`) and a metrics snapshot.
- Record the baseline (3 runs, median) and identify the **top two hotspots**.
- **Ordering is provisional:** if analysis dominates over `truePeak`, do S2 before S1.
- → checkpoint.

### S1 — True Peak kernel (expected biggest single win)

`truePeak()` calls `sin`/`cos` per tap, per phase, per sample (~48 trig calls/sample). Precompute
a polyphase kernel table once and use dot products; consider a shorter kernel.
**Acceptance:** True Peak identical within 0.01 dB; `truePeak` ≥ 5× faster; total time drops.
→ checkpoint.

### S2 — Single-pass analysis

`analyzeSource` makes ~8 STFT passes (octave energy, magnitude spectrum, band variation, band
transient, resonance, mid/side stereo, …). Compute **one FFT per frame** and derive all band
features from it.
**Acceptance:** one FFT pass; analysis metrics within tolerance; `analyzeSource` ≥ 3× faster.
→ checkpoint.

### S3 — Tighten the loudness loop

Cap iterations at 2–3, reuse the final measurement for the report, remove the duplicate
`analyzeLoudness` in `validateOutput`, and reuse a scratch buffer instead of copying the full
track per iteration (~95 MB per copy).
**Acceptance:** LUFS within 0.05; loop costs ≤ 2 full loudness measurements.
→ checkpoint.

### S4 — Parallelize independent work

Source vs reference analysis, per-channel stage processing, and per-band series via `std::thread`.
Keep results deterministic (no shared mutable state; fixed reduction order).
**Acceptance:** regression contract holds; wall-clock scales with cores.
→ checkpoint.

### S5 — Cache reference profiles

Use `ProfileCache` (and optionally an on-disk cache keyed by `ReferenceKey`) so repeated renders
skip reference analysis.
**Acceptance:** warm run with the same reference substantially faster; cache invalidates on
profile-version change.
→ checkpoint.

### S6 — Resolution policy (conditional)

Default analysis hop 1024 (half the frames) **only if** the regression contract holds; document
the accuracy/time trade-off. Skip if not needed.
→ checkpoint.

### S7 — Release + final validation

Ensure Release is the default for rendering; document the Debug-vs-Release factor (observed
~15 min vs ~6–7 min). Re-run the full 3-way comparison and the listening checklist.
→ checkpoint + final reflection.

### Speed log

| Step | Date | Median total | Top phase | Regression | Notes |
|---|---|---:|---|---|---|
| baseline | 2026-10-17 | 18m11s | truePeak | — | Release, 4.5-min track |
| S0+S1+S3 | 2026-10-17 | 1m57s | analyze 99.5s | held | True Peak kernel table; loudness-only loop |
| S4 | 2026-10-17 | 56s | analyze 35.6s | held | parallel feature passes (8 cores) |
| S4b+S3b+S6 | 2026-10-17 | **27.7s** | analyze 13.1s | held | parallel files; reuse measurements; hop 1024 |
