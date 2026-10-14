#include "masterizer/domain/analysis/Stft.h"

#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <algorithm>
#include <cstddef>

using masterizer::domain::analysis::averageMagnitudeSpectrum;
using masterizer::test::silence;
using masterizer::test::sine;

TEST_CASE(spectrum_of_silence_is_zero) {
    const auto data = silence(4096);
    const auto spectrum = averageMagnitudeSpectrum(data.data(), 4096, 1024, 256);
    CHECK(spectrum.size() == 513u);
    for (double value : spectrum) {
        CHECK_NEAR(value, 0.0, 1e-12);
    }
}

TEST_CASE(spectrum_peaks_at_the_tone_bin) {
    const auto data = sine(8192, 3000.0, 48000.0, 1.0f);
    const auto spectrum = averageMagnitudeSpectrum(data.data(), 8192, 1024, 256);
    const auto peak = std::max_element(spectrum.begin(), spectrum.end());
    CHECK(std::distance(spectrum.begin(), peak) == 64);
}

TEST_CASE(spectrum_of_short_signal_is_empty) {
    const auto data = silence(100);
    const auto spectrum = averageMagnitudeSpectrum(data.data(), 100, 1024, 256);
    CHECK(spectrum.empty());
}
