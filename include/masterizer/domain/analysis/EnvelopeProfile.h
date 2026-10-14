#pragma once

namespace masterizer::domain::analysis {

/// Time-domain dynamics estimate. `percussiveProminence` is a documented
/// estimate in [0, 1], not a source separation: it blends transient density
/// (onsets/second, saturating at 8/s) and crest factor (above 3 dB, saturating
/// at 15 dB), weighted 50/50.
struct EnvelopeProfile {
    double peak = 0.0;
    double rms = 0.0;
    double crestDb = 0.0;
    double transientDensity = 0.0;
    double percussiveProminence = 0.0;
};

/// Analyses one channel. Returns a default (zeroed) profile for invalid input.
EnvelopeProfile analyzeEnvelope(const float* samples, int count, double sampleRate);

}  // namespace masterizer::domain::analysis
