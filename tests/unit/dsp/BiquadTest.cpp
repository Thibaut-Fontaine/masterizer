#include "masterizer/domain/dsp/Biquad.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::BiquadCoefficients;
using masterizer::domain::dsp::BiquadState;
using masterizer::domain::dsp::processBiquad;

TEST_CASE(identity_biquad_passes_the_signal) {
    const BiquadCoefficients identity{1.0, 0.0, 0.0, 0.0, 0.0};
    BiquadState state;
    CHECK_NEAR(processBiquad(identity, state, 0.5), 0.5, 1e-12);
    CHECK_NEAR(processBiquad(identity, state, -0.25), -0.25, 1e-12);
}

TEST_CASE(pure_gain_biquad_scales_the_signal) {
    const BiquadCoefficients gain{0.5, 0.0, 0.0, 0.0, 0.0};
    BiquadState state;
    CHECK_NEAR(processBiquad(gain, state, 1.0), 0.5, 1e-12);
    CHECK_NEAR(processBiquad(gain, state, 0.4), 0.2, 1e-12);
}
