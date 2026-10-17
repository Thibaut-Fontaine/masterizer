#include "masterizer/domain/dsp/ParametricEq.h"

#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::domain::dsp::applyEqBands;
using masterizer::domain::dsp::EqBand;
using masterizer::domain::dsp::peakingEqCoefficients;
using masterizer::domain::dsp::rms;
using masterizer::test::sine;

TEST_CASE(peaking_eq_at_zero_gain_is_unity) {
    const auto coefficients = peakingEqCoefficients(48000.0, 1000.0, 1.4, 0.0);
    CHECK_NEAR(coefficients.b0, 1.0, 1e-9);
    CHECK_NEAR(coefficients.b1, coefficients.a1, 1e-12);
    CHECK_NEAR(coefficients.b2, coefficients.a2, 1e-12);
}

TEST_CASE(applying_no_bands_leaves_the_signal_unchanged) {
    auto data = sine(48000, 1000.0, 48000.0, 0.5f);
    const double before = rms(data.data(), 48000);
    applyEqBands(data.data(), 48000, 48000.0, {});
    CHECK_NEAR(rms(data.data(), 48000), before, 1e-9);
}

TEST_CASE(a_six_db_boost_doubles_the_band_rms) {
    auto data = sine(48000, 1000.0, 48000.0, 0.5f);
    const double before = rms(data.data(), 48000);
    applyEqBands(data.data(), 48000, 48000.0, {EqBand{1000.0, 1.4, 6.0}});
    const double after = rms(data.data(), 48000);
    CHECK_NEAR(after / before, 1.995, 0.05);
}

TEST_CASE(shelves_lift_lows_and_tame_highs) {
    namespace dsp = masterizer::domain::dsp;
    auto low = sine(48000, 40.0, 48000.0, 0.3f);
    const double lowBefore = rms(low.data(), 48000);
    applyEqBands(low.data(), 48000, 48000.0,
                 {dsp::EqBand{80.0, 0.707, 6.0, dsp::EqShape::LowShelf}});
    CHECK(rms(low.data(), 48000) > 1.5 * lowBefore);

    auto high = sine(48000, 16000.0, 48000.0, 0.3f);
    const double highBefore = rms(high.data(), 48000);
    applyEqBands(high.data(), 48000, 48000.0,
                 {dsp::EqBand{12000.0, 0.707, -6.0, dsp::EqShape::HighShelf}});
    CHECK(rms(high.data(), 48000) < 0.6 * highBefore);
}
