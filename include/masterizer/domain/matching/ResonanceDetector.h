#pragma once

#include "masterizer/domain/dsp/ParametricEq.h"

#include <vector>

namespace masterizer::domain::matching {

/// Conservative resonance-detection settings.
struct ResonanceSettings {
    double peakRatio = 8.0;    // peak vs local median power
    double cutDb = -4.0;       // attenuation applied per resonance
    double q = 8.0;            // narrow cut
    int maxCount = 4;          // at most this many cuts
    double minFrequencyHz = 40.0;
    int medianWindow = 16;     // half-window (bins) for the local median
};

/// Detects narrow peaks that stand well above their local neighbourhood in a
/// power spectrum, and returns bounded peaking cuts at those frequencies.
/// Only local maxima are considered, so broad tilts are ignored.
std::vector<dsp::EqBand> detectResonances(const std::vector<double>& powerSpectrum,
                                          double sampleRate, int fftSize,
                                          const ResonanceSettings& settings);

}  // namespace masterizer::domain::matching
