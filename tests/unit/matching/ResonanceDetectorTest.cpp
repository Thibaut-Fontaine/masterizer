#include "masterizer/domain/matching/ResonanceDetector.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <cstddef>
#include <vector>

using masterizer::domain::matching::detectResonances;
using masterizer::domain::matching::ResonanceSettings;

namespace {

constexpr double kSampleRate = 48000.0;
constexpr int kFftSize = 1024;
constexpr std::size_t kBins = 513;

std::vector<double> flatSpectrum(double level) {
    return std::vector<double>(kBins, level);
}

double binFrequency(int bin) {
    return static_cast<double>(bin) * kSampleRate / kFftSize;
}

}  // namespace

TEST_CASE(a_flat_spectrum_has_no_resonances) {
    const auto spectrum = flatSpectrum(1.0);
    CHECK(detectResonances(spectrum, kSampleRate, kFftSize, ResonanceSettings{}).empty());
}

TEST_CASE(a_narrow_spike_is_detected_and_cut) {
    auto spectrum = flatSpectrum(0.1);
    spectrum[64] = 10.0;
    const auto bands = detectResonances(spectrum, kSampleRate, kFftSize, ResonanceSettings{});
    CHECK(bands.size() == 1u);
    CHECK_NEAR(bands[0].frequencyHz, binFrequency(64), kSampleRate / kFftSize);
    CHECK(bands[0].gainDb < 0.0);
}

TEST_CASE(the_number_of_cuts_is_capped) {
    auto spectrum = flatSpectrum(0.1);
    for (int bin = 40; bin < 400; bin += 30) {
        spectrum[static_cast<std::size_t>(bin)] = 10.0;
    }
    const auto bands = detectResonances(spectrum, kSampleRate, kFftSize, ResonanceSettings{});
    CHECK(bands.size() <= 4u);
    CHECK(!bands.empty());
}
