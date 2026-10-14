#pragma once

namespace masterizer::domain::dsp {

/// True when every sample is finite (no NaN, no infinity). Empty input is finite.
bool allFinite(const float* samples, int count);

}  // namespace masterizer::domain::dsp
