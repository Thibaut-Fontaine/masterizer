#include "support/Signals.h"

#include "masterizer/domain/dsp/Peak.h"
#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <algorithm>
#include <cmath>

using masterizer::domain::dsp::peakAbs;
using masterizer::domain::dsp::rms;
using masterizer::test::impulse;
using masterizer::test::silence;
using masterizer::test::sine;
using masterizer::test::whiteNoise;

TEST_CASE(silence_is_all_zeros) {
    const auto data = silence(8);
    CHECK(data.size() == 8u);
    CHECK(std::all_of(data.begin(), data.end(), [](float sample) { return sample == 0.0f; }));
}

TEST_CASE(impulse_places_a_single_spike) {
    const auto data = impulse(8, 3, 0.9f);
    CHECK(data.size() == 8u);
    CHECK_NEAR(data[3], 0.9, 1e-6);
    double sum = 0.0;
    for (float sample : data) {
        sum += std::fabs(sample);
    }
    CHECK_NEAR(sum, 0.9, 1e-6);
}

TEST_CASE(sine_has_expected_rms) {
    const auto data = sine(48000, 1000.0, 48000.0, 1.0f);
    CHECK_NEAR(rms(data.data(), 48000), 0.70710678, 1e-3);
}

TEST_CASE(white_noise_is_bounded_and_deterministic) {
    const auto first = whiteNoise(1000, 1234u, 0.5f);
    const auto second = whiteNoise(1000, 1234u, 0.5f);
    CHECK(first == second);
    const double peak = peakAbs(first.data(), 1000);
    CHECK(peak <= 0.5 + 1e-6);
    CHECK(peak > 0.0);
}
