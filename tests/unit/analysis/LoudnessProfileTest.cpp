#include "masterizer/domain/analysis/LoudnessProfile.h"

#include "support/Buffers.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <cmath>

using masterizer::domain::analysis::analyzeLoudness;
using masterizer::test::silence;
using masterizer::test::sine;
using masterizer::test::stereoBuffer;

TEST_CASE(full_scale_sine_profile_has_expected_levels) {
    const auto data = sine(48000 * 5, 997.0, 48000.0, 1.0f);
    const auto profile = analyzeLoudness(masterizer::test::monoBuffer(data, 48000.0));
    CHECK_NEAR(profile.integratedLufs, -3.0103, 0.05);
    CHECK_NEAR(profile.truePeakDb, 0.0, 0.05);
    CHECK_NEAR(profile.samplePeakDb, 0.0, 0.05);
}

TEST_CASE(half_scale_stereo_true_peak_is_minus_six_db) {
    const auto data = sine(48000, 997.0, 48000.0, 0.5f);
    const auto profile = analyzeLoudness(stereoBuffer(data, data, 48000.0));
    CHECK_NEAR(profile.truePeakDb, -6.0206, 0.05);
}

TEST_CASE(silence_profile_reports_no_loudness) {
    const auto data = silence(48000);
    const auto profile = analyzeLoudness(masterizer::test::monoBuffer(data, 48000.0));
    CHECK(std::isinf(profile.integratedLufs));
    CHECK_NEAR(profile.truePeakDb, -144.0, 1e-9);
}
