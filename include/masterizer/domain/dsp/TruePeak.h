#pragma once

namespace masterizer::domain::dsp {

/// Inter-sample True Peak magnitude using 4x oversampling with a
/// Blackman-windowed sinc interpolation kernel. The un-supported edge margin
/// (kHalfTaps samples at each end) is excluded; the sampled peak is always
/// included, so the result never under-reads it. Returns 0 for empty input.
double truePeak(const float* samples, int count);

}  // namespace masterizer::domain::dsp
