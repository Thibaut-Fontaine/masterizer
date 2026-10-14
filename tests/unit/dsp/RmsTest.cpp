#include "masterizer/domain/dsp/Rms.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <cmath>

namespace {

using masterizer::domain::dsp::meanSquare;
using masterizer::domain::dsp::rms;

constexpr double kTwoPi = 2.0 * 3.14159265358979323846;

}  // namespace

TEST_CASE(rms_of_empty_buffer_is_zero) {
    CHECK_NEAR(rms(nullptr, 0), 0.0, 1e-12);
}

TEST_CASE(rms_of_constant_signal_equals_amplitude) {
    const float data[4] = {0.5f, 0.5f, 0.5f, 0.5f};
    CHECK_NEAR(rms(data, 4), 0.5, 1e-12);
}

TEST_CASE(rms_of_silence_is_zero) {
    const float data[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    CHECK_NEAR(rms(data, 8), 0.0, 1e-12);
}

TEST_CASE(rms_of_full_scale_sine_is_one_over_root_two) {
    float data[1000];
    for (int i = 0; i < 1000; ++i) {
        data[i] = static_cast<float>(std::sin(kTwoPi * static_cast<double>(i) / 1000.0));
    }
    CHECK_NEAR(rms(data, 1000), 0.7071067811865476, 1e-3);
}

TEST_CASE(mean_square_of_negative_signal_is_positive) {
    const float data[3] = {-0.25f, -0.25f, -0.25f};
    CHECK_NEAR(meanSquare(data, 3), 0.0625, 1e-12);
}
