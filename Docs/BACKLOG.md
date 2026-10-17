# Backlog

Prioritized work list. Each item has a definition of done. Update this file when an item
lands. Context: the engine currently matches a human master within ~0.4 dB on most bands and
matches crest/prominence, but it is **too flat** (over-limited to hit exact LUFS at −1 dBTP)
and **lacks air** above ~11.5 kHz.

## Metric roles (do not drift from this)

- True Peak: **hard constraint** (≤ −1.0 dBTP). Never match a hotter reference.
- LUFS: **soft target**. Approach it, but stop at the dynamics guard; report the shortfall.
- Crest / approx LRA: **guards**, not targets. If they fall too far below the reference, back off.
- Width: **optional/diagnostic**. Off for near-mono references; never chase per-band ratios.
- Lower spectral distance ≠ better sound.

## Immediate (next code session)

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

Target: **≤ 30 s** per track offline (stretch: ≤ 10 s), with identical numerical results
(within tolerance). Optimize in this order; measure first.

- **S0. Profile before optimizing.** Instrument `analyzeSource`, `truePeak`/`enforceTruePeak`,
  the loudness loop, and file I/O with timers (or `perf`). Add an optional `--timing` flag.
  *Done when:* we know the top two hotspots in wall-clock terms.
- **S1. True Peak kernel.** `truePeak()` calls `sin`/`cos` per tap, per phase, per sample
  (~48 trig calls/sample). Precompute a polyphase kernel table once and use dot products.
  Likely the dominant cost. *Done when:* same result within 0.01 dB and a large speedup.
- **S2. Single-pass analysis.** `analyzeSource` currently makes ~8 STFT passes (octave energy,
  magnitude spectrum, band variation, band transient, resonance, mid/side stereo, …). Compute
  one FFT per frame and derive all band features from it. *Done when:* one FFT pass and metrics
  match within tolerance.
- **S3. Closed-loop tightness.** Cap iterations at 2–3, reuse the final measurement for the
  report, and avoid the duplicate `analyzeLoudness` in `validateOutput`. *Done when:* the loop
  costs ≤ 2 full loudness measurements.
- **S4. Parallelize** independent work (source vs reference analysis, per-channel processing,
  per-band series) with `std::thread`; keep results deterministic.
- **S5. Cache reference profiles** (`ProfileCache` / on disk) so repeated renders skip reference
  analysis. *Done when:* a second run with the same reference is substantially faster.
- **S6. Resolution policy.** Default analysis hop 1024 (half the frames) after a quality check;
  document the trade-off.
- **S7. Release by default.** Ensure the CLI/engine are used in Release; document the
  Debug-vs-Release factor (the first Debug run was ~15 min vs ~6–7 min Release).
