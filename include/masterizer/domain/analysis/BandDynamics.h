#pragma once

#include <vector>

namespace masterizer::domain::analysis {

/// Per-band short-term dynamics: the coefficient of variation (std/mean) of the
/// per-frame band power. Returns exactly `bandCount()` values; higher means the
/// band varies more over time, 0 means steady or no data.
std::vector<double> bandVariation(const float* samples, int count, double sampleRate,
                                  int fftSize, int hop);

}  // namespace masterizer::domain::analysis
