#pragma once

namespace masterizer::domain::dsp {

/// Mean of the squared samples, i.e. mean-square energy. Returns 0 for an empty buffer.
double meanSquare(const float* samples, int count);

/// Root mean square level of a buffer, i.e. sqrt(meanSquare). Returns 0 when empty.
double rms(const float* samples, int count);

}  // namespace masterizer::domain::dsp
