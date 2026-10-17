#include "masterizer/application/LoudnessTargeter.h"

#include "masterizer/domain/analysis/LoudnessProfile.h"
#include "support/Buffers.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

using masterizer::application::targetLoudness;
using masterizer::domain::analysis::analyzeLoudness;
using masterizer::test::monoBuffer;
using masterizer::test::sine;

TEST_CASE(a_quiet_track_is_made_louder) {
    const auto data = sine(48000 * 5, 997.0, 48000.0, 0.1f);
    auto buffer = monoBuffer(data, 48000.0);
    const double before = analyzeLoudness(buffer).integratedLufs;
    const auto result = targetLoudness(buffer, -6.0, -1.0, 12.0, 0.0, true);
    CHECK(result.achievedLufs > before + 5.0);
    CHECK(result.truePeakDb <= -1.0 + 0.1);
}

TEST_CASE(no_gain_is_applied_when_the_target_is_lower) {
    const auto data = sine(48000 * 5, 997.0, 48000.0, 0.5f);
    auto buffer = monoBuffer(data, 48000.0);
    const auto result = targetLoudness(buffer, -30.0, 0.0, 12.0, 0.0, true);
    CHECK_NEAR(result.appliedGainDb, 0.0, 1e-9);
}

TEST_CASE(the_ceiling_limits_how_loud_it_can_get) {
    const auto data = sine(48000 * 5, 997.0, 48000.0, 0.5f);
    auto buffer = monoBuffer(data, 48000.0);
    const auto result = targetLoudness(buffer, 0.0, -6.0, 12.0, 0.0, true);
    CHECK(result.truePeakDb <= -6.0 + 0.1);
    CHECK(result.shortfallLu > 0.0);
}
