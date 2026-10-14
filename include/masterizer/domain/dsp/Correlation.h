#pragma once

namespace masterizer::domain::dsp {

/// Normalised correlation of two channels in [-1, 1].
/// Returns 0 when the buffers are empty, mismatched or either channel is silent.
double correlation(const float* left, const float* right, int count);

}  // namespace masterizer::domain::dsp
