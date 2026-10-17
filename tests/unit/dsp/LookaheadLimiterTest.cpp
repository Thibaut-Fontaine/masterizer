#include "masterizer/domain/dsp/LookaheadLimiter.h"

#include "masterizer/domain/dsp/Gain.h"
#include "masterizer/domain/dsp/Peak.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <cmath>

using masterizer::domain::dsp::applyLookaheadLimiter;
using masterizer::domain::dsp::dbToLinear;
using masterizer::domain::dsp::LookaheadLimiterSettings;
using masterizer::domain::dsp::peakAbs;
using masterizer::test::sine;

TEST_CASE(a_signal_below_the_ceiling_is_unchanged) {
    auto data = sine(48000, 1000.0, 48000.0, 0.5f);
    const double before = peakAbs(data.data(), 48000);
    applyLookaheadLimiter(data.data(), 48000, 48000.0, LookaheadLimiterSettings{});
    CHECK_NEAR(peakAbs(data.data(), 48000), before, 1e-6);
}

TEST_CASE(a_signal_above_the_ceiling_is_held_below_it) {
    auto data = sine(48000, 1000.0, 48000.0, 1.0f);
    applyLookaheadLimiter(data.data(), 48000, 48000.0, LookaheadLimiterSettings{});
    CHECK(peakAbs(data.data(), 48000) <= dbToLinear(-1.0) + 1e-3);
}

TEST_CASE(the_lookahead_limiter_stays_finite) {
    auto data = sine(48000, 4000.0, 48000.0, 1.0f);
    applyLookaheadLimiter(data.data(), 48000, 48000.0, LookaheadLimiterSettings{});
    for (float sample : data) {
        CHECK(std::isfinite(sample));
    }
}
