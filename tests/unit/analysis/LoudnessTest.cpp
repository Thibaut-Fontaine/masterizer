#include "masterizer/domain/analysis/Loudness.h"

#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <cmath>

using masterizer::domain::analysis::integratedLufs;
using masterizer::test::silence;
using masterizer::test::sine;

namespace {

constexpr int kFiveSeconds = 48000 * 5;

}  // namespace

TEST_CASE(integrated_loudness_of_silence_is_negative_infinity) {
    const auto data = silence(kFiveSeconds);
    const double result = integratedLufs(data.data(), kFiveSeconds, 48000.0);
    CHECK(std::isinf(result));
    CHECK(result < 0.0);
}

TEST_CASE(full_scale_997_hz_sine_is_minus_three_lufs) {
    const auto data = sine(kFiveSeconds, 997.0, 48000.0, 1.0f);
    CHECK_NEAR(integratedLufs(data.data(), kFiveSeconds, 48000.0), -3.0103, 0.05);
}

TEST_CASE(minus_twenty_dbfs_sine_is_minus_twenty_three_lufs) {
    const auto data = sine(kFiveSeconds, 997.0, 48000.0, 0.1f);
    CHECK_NEAR(integratedLufs(data.data(), kFiveSeconds, 48000.0), -23.0103, 0.05);
}

TEST_CASE(doubling_amplitude_adds_about_six_lu) {
    const auto quiet = sine(kFiveSeconds, 997.0, 48000.0, 0.25f);
    const auto loud = sine(kFiveSeconds, 997.0, 48000.0, 0.5f);
    const double quietLufs = integratedLufs(quiet.data(), kFiveSeconds, 48000.0);
    const double loudLufs = integratedLufs(loud.data(), kFiveSeconds, 48000.0);
    CHECK_NEAR(loudLufs - quietLufs, 6.0206, 0.05);
}

TEST_CASE(stereo_identical_channels_are_three_lu_louder) {
    const auto data = sine(kFiveSeconds, 997.0, 48000.0, 0.5f);
    const double single = integratedLufs(data.data(), kFiveSeconds, 48000.0);
    const double stereo = integratedLufs(data.data(), data.data(), kFiveSeconds, 48000.0);
    CHECK_NEAR(stereo - single, 3.0103, 0.05);
}
