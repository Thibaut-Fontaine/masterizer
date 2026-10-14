#include "masterizer/domain/dsp/Peak.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::peakAbs;

TEST_CASE(peak_of_empty_buffer_is_zero) {
    CHECK_NEAR(peakAbs(nullptr, 0), 0.0, 1e-12);
}

TEST_CASE(peak_reports_largest_magnitude) {
    const float data[3] = {-0.5f, 0.25f, 0.1f};
    CHECK_NEAR(peakAbs(data, 3), 0.5, 1e-12);
}

TEST_CASE(peak_of_silence_is_zero) {
    const float data[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    CHECK_NEAR(peakAbs(data, 4), 0.0, 1e-12);
}
