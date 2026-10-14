#pragma once

#include <vector>

namespace masterizer::domain::analysis {

/// Averaged linear magnitude spectrum over Hann-windowed frames, covering bins
/// 0..fftSize/2 inclusive. `fftSize` must be a power of two and >= 2.
/// Returns an empty vector for invalid parameters or when the signal is shorter
/// than a single frame.
std::vector<double> averageMagnitudeSpectrum(const float* samples, int count,
                                             int fftSize, int hop);

}  // namespace masterizer::domain::analysis
