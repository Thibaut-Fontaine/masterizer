#pragma once

namespace masterizer::domain::dsp {

/// Inclusive decibel range used to bound proposed gains.
struct GainBounds {
    double minDb;
    double maxDb;
};

/// Linear gain factor for a value in decibels.
double dbToLinear(double decibels);

/// Decibels for a linear gain. Returns -infinity for non-positive gains.
double linearToDb(double linearGain);

/// Clamps a gain in dB to the inclusive bounds.
double clampGainDb(double gainDb, const GainBounds& bounds);

/// True when the gain is finite and lies within the inclusive bounds.
bool withinGainBounds(double gainDb, const GainBounds& bounds);

}  // namespace masterizer::domain::dsp
