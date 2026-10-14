#include "masterizer/domain/dsp/Biquad.h"

namespace masterizer::domain::dsp {

double processBiquad(const BiquadCoefficients& coefficients, BiquadState& state, double input) {
    const double output = coefficients.b0 * input + coefficients.b1 * state.x1 +
                          coefficients.b2 * state.x2 - coefficients.a1 * state.y1 -
                          coefficients.a2 * state.y2;
    state.x2 = state.x1;
    state.x1 = input;
    state.y2 = state.y1;
    state.y1 = output;
    return output;
}

}  // namespace masterizer::domain::dsp
