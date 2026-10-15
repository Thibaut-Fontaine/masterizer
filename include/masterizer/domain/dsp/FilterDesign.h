#pragma once

#include "masterizer/domain/dsp/Biquad.h"

namespace masterizer::domain::dsp {

/// RBJ 2nd-order low-pass, normalised so a0 = 1.
BiquadCoefficients lowpassCoefficients(double sampleRate, double frequencyHz, double q);

/// RBJ 2nd-order high-pass, normalised so a0 = 1.
BiquadCoefficients highpassCoefficients(double sampleRate, double frequencyHz, double q);

/// RBJ 2nd-order band-pass (constant peak gain), normalised so a0 = 1.
BiquadCoefficients bandpassCoefficients(double sampleRate, double frequencyHz, double q);

}  // namespace masterizer::domain::dsp
