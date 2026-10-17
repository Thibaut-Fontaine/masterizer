#pragma once

#include "masterizer/domain/dsp/Biquad.h"

#include <vector>

namespace masterizer::domain::dsp {

/// Shape of an EQ band.
enum class EqShape {
    Peaking,
    LowShelf,
    HighShelf,
};

/// One EQ band.
struct EqBand {
    double frequencyHz;
    double q;
    double gainDb;
    EqShape shape = EqShape::Peaking;
};

/// RBJ cookbook peaking-EQ coefficients, normalised so a0 = 1.
BiquadCoefficients peakingEqCoefficients(double sampleRate, double frequencyHz, double q,
                                         double gainDb);

/// Applies cascaded peaking bands in place to one channel.
void applyEqBands(float* samples, int count, double sampleRate,
                  const std::vector<EqBand>& bands);

}  // namespace masterizer::domain::dsp
