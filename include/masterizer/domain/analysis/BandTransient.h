#pragma once

#include <vector>

namespace masterizer::domain::analysis {

/// Per-band transient behaviour, each vector having `bandCount()` entries.
/// `density` is onsets per second (a per-frame band-energy jump above ~6 dB);
/// `crest` is max/mean of the per-frame band power (a temporal crest proxy).
struct BandTransientProfile {
    std::vector<double> density;
    std::vector<double> crest;
};

/// Analyses one channel. Empty input yields zeroed vectors.
BandTransientProfile analyzeBandTransient(const float* samples, int count, double sampleRate,
                                          int fftSize, int hop);

}  // namespace masterizer::domain::analysis
