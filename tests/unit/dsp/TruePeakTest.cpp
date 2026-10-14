#include "masterizer/domain/dsp/TruePeak.h"

#include "masterizer/domain/dsp/Peak.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <cmath>
#include <vector>

using masterizer::domain::dsp::peakAbs;
using masterizer::domain::dsp::truePeak;
using masterizer::test::silence;
using masterizer::test::sine;

namespace {

constexpr double kPi = 3.14159265358979323846;

}  // namespace

TEST_CASE(true_peak_of_silence_is_zero) {
    const auto data = silence(64);
    CHECK_NEAR(truePeak(data.data(), static_cast<int>(data.size())), 0.0, 1e-12);
}

TEST_CASE(true_peak_of_impulse_equals_amplitude) {
    std::vector<float> data(64, 0.0f);
    data[10] = 0.9f;
    CHECK_NEAR(truePeak(data.data(), 64), 0.9, 1e-6);
}

TEST_CASE(true_peak_catches_inter_sample_peak) {
    const int count = 4096;
    std::vector<float> data(static_cast<std::size_t>(count), 0.0f);
    for (int i = 0; i < count; ++i) {
        // fs/4 sine at 45 degrees: every sample is +/-0.7071, but the true peak is 1.0.
        data[static_cast<std::size_t>(i)] =
            static_cast<float>(std::sin(kPi * 0.5 * i + kPi * 0.25));
    }
    CHECK_NEAR(peakAbs(data.data(), count), 0.70710678, 1e-4);
    CHECK_NEAR(truePeak(data.data(), count), 1.0, 5e-3);
}

TEST_CASE(true_peak_is_at_least_sample_peak) {
    const auto data = sine(48000, 997.0, 48000.0, 1.0f);
    const double samplePeak = peakAbs(data.data(), 48000);
    const double peak = truePeak(data.data(), 48000);
    CHECK(peak >= samplePeak - 1e-9);
    CHECK(peak <= 1.05);
}
