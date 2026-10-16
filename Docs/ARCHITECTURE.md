# Architecture

Layered MVC + Ports & Adapters. The domain is pure C++20 with no JUCE and is fully
unit-tested; everything host- or platform-specific is an adapter added later.

```text
app/ (masterizer_cli)         offline render tool
   |
application/                 controllers / use-cases: ProcessingChain, ChainStages,
   |                         DynamicsStages, TruePeakGuard, OutputValidator, ChainReporter,
   |                         MonitoringController
   |
domain/                      pure logic, no JUCE
   |-- common/                Types, OctaveBands
   |-- audio/                 AudioBuffer, DecodedAudio, IAudioDecoder (port), ops
   |-- dsp/                   measurement + processors (Rms, TruePeak, Biquad, Compressor,
   |                          MultibandCompressor, Saturator, TransientShaper, MonoBass,
   |                          ParametricEq, DynamicEq, StereoWidth, Limiter, Gain, ...)
   |-- analysis/              STFT/power spectrum, octave/band/stereo/loudness profiles,
   |                          SourceAnalyzer, AnalysisProfile (versioned)
   |-- matching/              ProfileDifference, MatchingEngine, EqDesigner, MatchIntensity,
   |                          ChainCoordinator, DynamicsPlanner, ResonanceDetector, MixAdvisor
   |-- state/                 ReferenceKey, ProfileCache, ReferenceManager
   |
io/                          WavFile (WAV read/write), AudioFile (WAV/FLAC/MP3 read)
   |
third_party/dr_libs/         vendored single-file MP3/FLAC decoders (public domain / MIT-0)
   |
plugin/ (future)             JUCE VST3 adapter: PluginProcessor, PluginEditor
```

## Targets

- `masterizer_core` — static library (all of the above except `app/`).
- `masterizer_tests` — the unit + integration test runner (the executable tests use).
- `masterizer_cli` — offline renderer.

## Contracts

- `AnalysisProfile` — immutable, `ProfileVersion`-stamped; produced by `analyzeSource`.
- `ProfileDifference` — level-normalised Source-vs-Reference comparison.
- `CorrectionProposal` — bounded, confidence-tagged correction (Stage 5).
- `MixSuggestion` — advice-only source-level suggestion (Stage 5b).
- `ChainSettings` / `ChainResult` — full-chain configuration and output.
- `OutputReport` — true peak, LUFS, target shortfall, ceiling flag.

## Boundaries

- The domain must never include JUCE or perform file I/O; decoding is behind `IAudioDecoder`.
- The UI must never touch DSP internals directly.
- The audio callback (future) must not allocate, lock, log or do I/O.
- Analysis and matching are offline; processors run with their own detectors.
