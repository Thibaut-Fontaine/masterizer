#pragma once

#include "masterizer/domain/dsp/Biquad.h"
#include "masterizer/domain/dsp/ParametricEq.h"

#include <vector>

namespace masterizer::domain::dsp {

/// Stateful version of `applyEqBands` for block-based processing. The filter
/// state survives across calls, so splitting a buffer into host blocks does not
/// reset the EQ.
class EqProcessor {
public:
    EqProcessor(double sampleRate, const std::vector<EqBand>& bands);
    void process(float* samples, int count);

private:
    std::vector<BiquadCoefficients> coefficients_;
    std::vector<BiquadState> states_;
};

}  // namespace masterizer::domain::dsp
