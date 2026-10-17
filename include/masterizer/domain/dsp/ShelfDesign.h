#pragma once

#include "masterizer/domain/dsp/Biquad.h"

namespace masterizer::domain::dsp {

/// RBJ low-shelf, normalised so a0 = 1.
BiquadCoefficients lowShelfCoefficients(double sampleRate, double frequencyHz, double q,
                                        double gainDb);

/// RBJ high-shelf, normalised so a0 = 1.
BiquadCoefficients highShelfCoefficients(double sampleRate, double frequencyHz, double q,
                                         double gainDb);

}  // namespace masterizer::domain::dsp
