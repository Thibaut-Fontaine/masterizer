#pragma once

#include <vector>

namespace masterizer::domain::analysis {

/// Per-octave-band mean-square energy from the signal's power spectrum.
/// Returns exactly `bandCount()` values (all zero when there is no usable data),
/// indexed by `static_cast<int>(OctaveBand)`.
std::vector<double> octaveBandEnergies(const float* samples, int count,
                                       double sampleRate, int fftSize, int hop);

}  // namespace masterizer::domain::analysis
