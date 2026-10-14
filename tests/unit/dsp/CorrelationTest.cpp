#include "masterizer/domain/dsp/Correlation.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::correlation;

TEST_CASE(correlation_of_identical_channels_is_one) {
    const float data[3] = {0.3f, -0.2f, 0.5f};
    CHECK_NEAR(correlation(data, data, 3), 1.0, 1e-12);
}

TEST_CASE(correlation_of_inverted_channels_is_minus_one) {
    const float left[2] = {0.5f, -0.25f};
    const float right[2] = {-0.5f, 0.25f};
    CHECK_NEAR(correlation(left, right, 2), -1.0, 1e-12);
}

TEST_CASE(correlation_of_orthogonal_channels_is_zero) {
    const float left[2] = {1.0f, 0.0f};
    const float right[2] = {0.0f, 1.0f};
    CHECK_NEAR(correlation(left, right, 2), 0.0, 1e-12);
}

TEST_CASE(correlation_of_silence_is_zero) {
    const float data[2] = {0.0f, 0.0f};
    CHECK_NEAR(correlation(data, data, 2), 0.0, 1e-12);
}
