#pragma once

#include "masterizer/domain/dsp/Biquad.h"

namespace masterizer::domain::dsp {

/// Stage-1 high-shelf pre-filter of the BS.1770-4 K-weighting curve.
BiquadCoefficients kWeightingShelf(double sampleRate);

/// Stage-2 RLB high-pass of the BS.1770-4 K-weighting curve.
BiquadCoefficients kWeightingHighPass(double sampleRate);

}  // namespace masterizer::domain::dsp
