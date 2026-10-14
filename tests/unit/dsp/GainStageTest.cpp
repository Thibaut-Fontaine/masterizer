#include "masterizer/domain/dsp/GainStage.h"

#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::applyGain;
using masterizer::domain::dsp::rms;
using masterizer::test::sine;

TEST_CASE(gain_of_zero_decibels_is_identity) {
    auto data = sine(1000, 1000.0, 48000.0, 0.5f);
    const double before = rms(data.data(), 1000);
    applyGain(data.data(), 1000, 0.0);
    CHECK_NEAR(rms(data.data(), 1000), before, 1e-9);
}

TEST_CASE(six_db_gain_doubles_the_signal) {
    auto data = sine(1000, 1000.0, 48000.0, 0.25f);
    const double before = rms(data.data(), 1000);
    applyGain(data.data(), 1000, 6.0206);
    CHECK_NEAR(rms(data.data(), 1000) / before, 2.0, 1e-3);
}
