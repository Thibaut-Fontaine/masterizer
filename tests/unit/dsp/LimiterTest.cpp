#include "masterizer/domain/dsp/Limiter.h"

#include "masterizer/domain/dsp/Gain.h"
#include "masterizer/domain/dsp/Peak.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::applyLimiter;
using masterizer::domain::dsp::dbToLinear;
using masterizer::domain::dsp::LimiterSettings;
using masterizer::domain::dsp::peakAbs;
using masterizer::test::sine;

TEST_CASE(signal_below_the_ceiling_is_unchanged) {
    auto data = sine(48000, 1000.0, 48000.0, 0.5f);
    const double before = peakAbs(data.data(), 48000);
    applyLimiter(data.data(), 48000, 48000.0, LimiterSettings{});
    CHECK_NEAR(peakAbs(data.data(), 48000), before, 1e-6);
}

TEST_CASE(signal_above_the_ceiling_is_limited_to_it) {
    auto data = sine(48000, 1000.0, 48000.0, 1.0f);
    applyLimiter(data.data(), 48000, 48000.0, LimiterSettings{});
    const double ceiling = dbToLinear(-1.0);
    CHECK(peakAbs(data.data(), 48000) <= ceiling + 1e-3);
    CHECK(peakAbs(data.data(), 48000) > 0.8 * ceiling);
}
