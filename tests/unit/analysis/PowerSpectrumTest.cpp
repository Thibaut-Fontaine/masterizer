#include "masterizer/domain/analysis/PowerSpectrum.h"

#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <algorithm>
#include <cmath>

using masterizer::domain::analysis::averagePowerSpectrum;
using masterizer::test::silence;
using masterizer::test::sine;

TEST_CASE(power_spectrum_of_silence_is_zero) {
    const auto data = silence(4096);
    const auto spectrum = averagePowerSpectrum(data.data(), 4096, 1024, 256);
    CHECK(spectrum.size() == 513u);
    for (double value : spectrum) {
        CHECK_NEAR(value, 0.0, 1e-12);
    }
}

TEST_CASE(power_spectrum_peaks_at_the_tone_bin) {
    const auto data = sine(8192, 3000.0, 48000.0, 1.0f);
    const auto spectrum = averagePowerSpectrum(data.data(), 8192, 1024, 256);
    const auto peak = std::max_element(spectrum.begin(), spectrum.end());
    CHECK(std::distance(spectrum.begin(), peak) == 64);
}

TEST_CASE(power_spectrum_is_non_negative_and_finite) {
    const auto data = sine(8192, 1000.0, 48000.0, 0.5f);
    const auto spectrum = averagePowerSpectrum(data.data(), 8192, 1024, 256);
    for (double value : spectrum) {
        CHECK(std::isfinite(value) && value >= 0.0);
    }
}
