#pragma once

namespace masterizer::domain::dsp {

/// Maximum absolute sample value. Returns 0 for an empty buffer.
double peakAbs(const float* samples, int count);

}  // namespace masterizer::domain::dsp
