#pragma once

namespace masterizer::domain::dsp {

/// One dynamic-EQ band: attenuates its frequency region only while that region
/// exceeds the threshold. Below threshold the signal is left untouched.
struct DynamicEqBandSettings {
    double frequencyHz = 200.0;
    double q = 1.0;
    double thresholdDb = -18.0;
    double maxCutDb = 6.0;
    double attackSeconds = 0.01;
    double releaseSeconds = 0.1;
};

void applyDynamicEqBand(float* samples, int count, double sampleRate,
                        const DynamicEqBandSettings& settings);

}  // namespace masterizer::domain::dsp
