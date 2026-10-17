# DSP definitions

Documented measurement and processing definitions. Source/Reference always use the same
settings. All thresholds in the code are initial values, not calibrated constants.

## Analysis resolution

- STFT: symmetric Hann window, default `fftSize = 2048`, `hop = 512`, averaged over frames.
- Octave bands: Sub <45, Low 45-90, UpperLow 90-180, LowMid 180-360, Mid 360-720,
  UpperMid 720-1440, HighMid 1440-2880, Presence 2880-5760, High 5760-11520, VeryHigh >11520 Hz.
  Band centres are geometric means (e.g. LowMid ~254.6 Hz). Sub is open below, VeryHigh open above.

## Level and dynamics

- **RMS / mean-square**: mean of squared samples over the buffer.
- **Peak**: max |x|.
- **True Peak**: 4x oversampling with a Blackman-windowed sinc kernel; the un-supported edge
  margin (6 samples each end) is excluded and the sampled peak is always included, so it never
  under-reads. Reported in dBTP.
- **Integrated LUFS**: BS.1770-4. K-weighting = high-shelf pre-filter + RLB high-pass, designed
  analytically per sample rate (reproduces the ITU 48 kHz coefficients exactly). 400 ms blocks,
  100 ms hop, per-block channel energy summed, absolute gate -70 LUFS and relative gate -10 LU,
  -0.691 dB offset. A full-scale 997 Hz sine reads -3.0103 LUFS.
- **Crest factor (global)**: 20*log10(peak/rms), dB.
- **Band variation**: coefficient of variation (std/mean) of the per-frame band power; a
  short-term dynamics measure (high = more movement).
- **Band transient density**: per-second count of per-frame band-energy jumps above ~+6 dB.
- **Band crest (temporal proxy)**: max/mean of per-frame band power.
- **Persistence**: fraction of frames whose band energy exceeds 1.5x the band mean.
- **Peakiness**: max/median bin power inside a band (narrow-peak indicator).
- **Percussive prominence (estimate)**: 0.5 * min(1, density/8) + 0.5 * min(1, (crest-3)/15),
  in [0, 1]. An estimate, not source separation.

## Stereo

- **Mid/Side**: M = (L+R)/2, S = (L-R)/2.
- **Width**: side energy / (mid + side) over total band energy, in [0, 1].
- **Correlation**: normalised (sum LR / sqrt(sum L^2 * sum R^2)), in [-1, 1]; 0 for silence.

## Processing

- **Mono/M-S**: 2nd-order RBJ high-pass on the Side (4th-order when strict), default transition
  95 Hz. Removes low side content; the Mid keeps the low end centred.
- **Parametric EQ**: RBJ peaking filters, Q 1.4 per octave band.
- **Saturation**: tanh soft clip at drive g, divided by g so small-signal gain is preserved
  (level-compensated); optional parallel mix.
- **Compressor**: feed-forward, one-pole detector, soft knee, configurable ratio/attack/release/
  makeup. Multiband version splits with an LR4 crossover.
- **Transient shaper**: dual envelope; strong attacks weighted toward the attack gain, sustained
  parts toward the sustain gain.
- **De-resonator**: narrow peaks above a local median in the power spectrum become bounded cuts.
- **Dynamic EQ**: band split; the band is attenuated only while its envelope exceeds threshold.
- **Stereo width**: Side scaled by a factor; `widthScaleFor` solves the factor for a target.
- **Limiter**: instant attack, exponential release, sample-peak ceiling (default -1.0 dB).
- **True Peak guard**: single static scale-down so the render's True Peak is at or below ceiling.
- **Loudness targeting**: after processing, gain is raised toward the reference integrated LUFS
  (capped at +12 dB by default), then the limiter and True Peak guard apply; any remaining
  difference is reported as a shortfall rather than over-limiting.
- **Tape stage (opt-in)**: subtle low shelf (~70 Hz head bump) -> soft saturation
  (level-compensated) -> high shelf (~14 kHz roll-off), with a wet/dry mix. The defaults are
  deliberately tiny. It is not a physical tape model: no wow/flutter, crosstalk, hiss, bias or
  tape formulation.
- **Dynamics planner**: compares per-band variation; enables gentle glue (ratio 1.5-2) and,
  at larger gaps, multiband compression. Release can follow BPM (default 0.25 beat), clamped.
