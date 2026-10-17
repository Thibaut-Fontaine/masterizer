# Diagnostic — Dax J "Femme Fatale" (step 0)

Inputs: `PREMASTER.wav` (mix), `MASTER.wav` (human master), `OURS6.wav` (our render,
−10.61 LUFS / −1.00 dBTP). Method: section-wise 10 s windows, level-matched delta
(MASTER − PREMASTER at MASTER loudness), delta spectral flatness, per-band temporal crest.

## Findings

### 1. The human processing is concentrated in the mids/highs, not the lows
Level-matched delta energy relative to MASTER, per band:

| Band | Δ/MASTER |
|---|---:|
| Sub | −21.1 dB |
| Low | −28.0 |
| ULow | −30.5 |
| LowMid | −27.6 |
| Mid | −17.7 |
| UMid | −22.8 |
| **HMid 1.4–2.8k** | **−13.1** |
| **Presence 2.8–5.8k** | **−12.9** |
| **High 5.8–11.5k** | **−12.1** |
| **VHigh >11.5k** | **−13.9** |

The delta is ~8–18 dB hotter in the upper bands than in the lows.

### 2. Dynamics are distributed differently
Per-band temporal crest (attack proxy), PRE / MASTER / OURS:

| Band | PRE | MASTER | OURS |
|---|---:|---:|---:|
| Sub | 6.9 | 7.2 | **6.2** |
| Low | 7.2 | 6.5 | **5.5** |
| ULow | 11.7 | 10.8 | **8.8** |
| Mid | 13.1 | 10.9 | 12.2 |
| UMid | 8.2 | 7.6 | 8.1 |
| HMid | 9.1 | **7.4** | 9.5 |
| Presence | 9.2 | **8.9** | **11.8** |
| High | 11.8 | **10.0** | **13.3** |
| VHigh | 16.6 | **15.6** | **17.8** |

The human master **compresses the upper mids/highs more** than we do, and the lows less.
We do the opposite: heavy low-end crest reduction, comparatively open highs.

### 3. We are globally over-compressed
Global crest per 10 s window: PRE ≈ 13–15 dB, MASTER ≈ 9.8–12.6, OURS ≈ 8.8–10.9.
OURS is **~1.2 dB flatter than MASTER in every section** — the user's "master has more
movement" is correct.

### 4. The HMid difference flips by section; Presence does not
`OURS − MASTER` at 1.4–2.8 kHz flips sign (sign balance −8 across 27 windows), i.e. it is
**section-dependent** → dynamic/multiband behaviour, not a static EQ.
Presence is consistently **positive** (+0.2 to +0.6 dB in 20/27 windows) → a static excess in ours.

### 5. Air gap
VHigh is **−1.75 dB** in ours. The human master adds ~+1.6 dB relative air; we cannot (edge band).

### 6. Saturation / exciter — weak evidence
Delta spectral flatness (1–10 kHz) is **0.762** vs the master's **0.736** — only marginally
flatter. This is **not** strong evidence of broadband harmonic generation. The pattern is much
better explained by **multiband/dynamic control in the upper bands + static presence/air EQ +
over-limiting**, not by a saturator or exciter. Do **not** add saturation on this basis.

## Interpretation
The master's "lead slightly more forward in the mids/highs" is most consistent with:
1. **denser dynamics in 1.4–11.5 kHz** (lower per-band crest) — multiband compression / dynamic EQ;
2. **a static presence trim** (ours is +0.4 dB) and **air lift** (ours is −1.75 dB);
3. **less low-end over-compression** in ours.

## Decision for steps 1–8 (priority)
1. **Dynamics guard** — reduce our global over-compression (crest −1.2 dB).
2. **Edge-band shelves** — close the air gap.
3. **Tape HF roll-off** — stop fighting the air.
4. **Dynamic EQ / multiband in HMid/Presence** — justified by §2 and §4 (not by spectrum alone).
5. **Finer EQ** — presence is a consistent +0.2…+0.6 dB static excess.
6. **Real per-band correlation** — metric correctness.
7. **Near-mono guard** — avoid width chasing on this near-mono track.
8. **Look-ahead limiter** — quality/attacks; global over-compression is confirmed.

Listening remains the final arbiter; these are metric proxies.

## Follow-up: the 8-12 kHz gap (fixed)

The user reported the highs as clearly stronger on MASTER. Third-octave measurement of OURS7
showed the gap was concentrated at **7.9-10 kHz (+1.5 to +1.9 dB)**, not above 12 kHz.

Cause: the air shelf sat at **12 kHz**, so it fixed >12 kHz but under-corrected 8-12 kHz; the
High octave band (5.8-11.5 k, +0.65 dB) was rejected by the confidence gate (0.108 < 0.2), so
nothing corrected it.

Fix: move the VHigh high shelf from 12 kHz to **8 kHz**. Result (OURS8):
7.9 k +1.47 → **+0.41**; 10 k +1.93 → **−0.28**; High +0.65 → **+0.16**; VHigh +0.26 → **−0.17**.

With `--ceiling -0.1 dBTP` (OURS9) the level also matches: **−10.54 LUFS** vs −10.50, crest
10.58 vs 10.74 (−0.16 dB), TP −0.10 dBTP. The default ceiling stays −1.0 (spec), where the
crest guard leaves a ~1 LU shortfall.

