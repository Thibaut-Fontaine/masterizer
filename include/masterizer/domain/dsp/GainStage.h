#pragma once

namespace masterizer::domain::dsp {

/// Simple in-place gain stage.
void applyGain(float* samples, int count, double gainDb);

}  // namespace masterizer::domain::dsp
