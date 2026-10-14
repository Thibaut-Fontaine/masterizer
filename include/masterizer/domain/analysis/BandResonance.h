#pragma once

#include <vector>

namespace masterizer::domain::analysis {

/// Per-band resonance indicators, each vector having `bandCount()` entries.
/// `persistence` is the fraction of frames whose band energy exceeds 1.5x the
/// band mean. `peakiness` is the max/median bin power inside the band (a high
/// value indicates a narrow dominant peak).
struct BandResonanceProfile {
    std::vector<double> persistence;
    std::vector<double> peakiness;
};

/// Analyses one channel. Empty input yields zeroed vectors.
BandResonanceProfile analyzeBandResonance(const float* samples, int count, double sampleRate,
                                          int fftSize, int hop);

}  // namespace masterizer::domain::analysis
