#pragma once

namespace masterizer::domain::dsp {

/// Saturation settings. `driveDb <= 0` leaves the signal unchanged.
struct SaturationSettings {
    double driveDb = 0.0;
    double mix = 1.0;  // parallel blend, 0..1
};

/// tanh soft-clipping saturation. The shaper is divided by its small-signal gain
/// so quiet material is level-preserved while peaks are gently compressed.
void applySaturation(float* samples, int count, const SaturationSettings& settings);

}  // namespace masterizer::domain::dsp
