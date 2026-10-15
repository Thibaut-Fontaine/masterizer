#pragma once

namespace masterizer::domain::dsp {

/// Feed-forward compressor settings.
struct CompressorSettings {
    double thresholdDb = -18.0;
    double ratio = 2.0;
    double attackSeconds = 0.01;
    double releaseSeconds = 0.1;
    double kneeDb = 6.0;
    double makeupDb = 0.0;
};

/// Detector state (one per channel).
struct CompressorState {
    double envelope = 0.0;
};

/// Compresses a single sample, updating `state`. Returns the processed sample.
double compressSample(double input, const CompressorSettings& settings, double sampleRate,
                      CompressorState& state);

/// Applies the compressor in place to one channel.
void applyCompressor(float* samples, int count, double sampleRate,
                     const CompressorSettings& settings);

}  // namespace masterizer::domain::dsp
