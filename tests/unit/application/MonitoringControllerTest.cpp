#include "masterizer/application/MonitoringController.h"

#include "support/Buffers.h"
#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::application::differenceSignal;
using masterizer::application::loudnessMatchGainDb;
using masterizer::domain::audio::AudioBuffer;
using masterizer::test::monoBuffer;

TEST_CASE(match_gain_is_the_loudness_difference) {
    CHECK_NEAR(loudnessMatchGainDb(-9.0, -6.0), -3.0, 1e-12);
    CHECK_NEAR(loudnessMatchGainDb(-6.0, -9.0), 3.0, 1e-12);
    CHECK_NEAR(loudnessMatchGainDb(-1.0 / 0.0, -6.0), 0.0, 1e-12);
}

TEST_CASE(delta_of_identical_signals_is_silence) {
    const auto signal = monoBuffer({0.1f, -0.2f, 0.3f}, 48000.0);
    const auto delta = differenceSignal(signal, signal);
    for (float sample : delta.channels[0]) {
        CHECK_NEAR(sample, 0.0, 1e-12);
    }
}

TEST_CASE(delta_is_processed_minus_original) {
    const auto processed = monoBuffer({1.0f, 2.0f, 3.0f}, 48000.0);
    const auto original = monoBuffer({0.5f, 0.5f, 0.5f}, 48000.0);
    const auto delta = differenceSignal(processed, original);
    CHECK_NEAR(delta.channels[0][0], 0.5, 1e-6);
    CHECK_NEAR(delta.channels[0][1], 1.5, 1e-6);
    CHECK_NEAR(delta.channels[0][2], 2.5, 1e-6);
}
