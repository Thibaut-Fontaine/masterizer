#include "masterizer/application/LoudnessTargeter.h"

#include "masterizer/domain/analysis/LoudnessProfile.h"
#include "masterizer/domain/dsp/CrestFactor.h"
#include "support/Buffers.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/SignalsProgram.h"
#include "support/TestRegistry.h"

using masterizer::application::targetLoudness;
using masterizer::domain::analysis::analyzeLoudness;
using masterizer::test::monoBuffer;
using masterizer::test::sine;
using masterizer::test::transientRich;

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

TEST_CASE(the_closed_loop_reaches_a_reachable_target) {
    const auto data = sine(48000 * 5, 997.0, 48000.0, 0.1f);
    auto buffer = monoBuffer(data, 48000.0);
    const auto result = targetLoudness(buffer, -12.0, -1.0, 12.0, 0.0, true);
    CHECK_NEAR(result.achievedLufs, -12.0, 0.3);
    CHECK_NEAR(result.shortfallLu, 0.0, 1e-9);
}

TEST_CASE(the_crest_guard_prevents_over_compression) {
    const int frames = 48000 * 5;
    const auto data = transientRich(frames, 4000, 0.6, 0.8f);
    auto buffer = monoBuffer(data, 48000.0);
    const double referenceCrest = masterizer::domain::dsp::crestFactorDb(data.data(), frames);
    const auto result = targetLoudness(buffer, -6.0, -1.0, 12.0, 0.0, true, true,
                                       referenceCrest, 0.5);
    const double crestAfter = masterizer::domain::dsp::crestFactorDb(
        buffer.channels[0].data(), frames);
    CHECK(crestAfter >= referenceCrest - 0.5 - 1e-6);
    CHECK(result.shortfallLu > 0.0);
}
