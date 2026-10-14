#include "masterizer/domain/analysis/OctaveProfile.h"

#include "masterizer/domain/analysis/PowerSpectrum.h"
#include "masterizer/domain/common/OctaveBands.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/SignalsProgram.h"
#include "support/TestRegistry.h"

#include <algorithm>
#include <cstddef>

using masterizer::domain::analysis::octaveBandEnergies;
using masterizer::domain::bandCount;
using masterizer::test::bassHeavy;
using masterizer::test::silence;
using masterizer::test::sine;

namespace {

std::size_t dominantBand(const std::vector<double>& energies) {
    return static_cast<std::size_t>(
        std::distance(energies.begin(), std::max_element(energies.begin(), energies.end())));
}

}  // namespace

TEST_CASE(octave_profile_has_one_energy_per_band) {
    const auto data = silence(8192);
    const auto energies = octaveBandEnergies(data.data(), 8192, 48000.0, 1024, 256);
    CHECK(energies.size() == static_cast<std::size_t>(bandCount()));
    for (double energy : energies) {
        CHECK_NEAR(energy, 0.0, 1e-12);
    }
}

TEST_CASE(one_khz_tone_lands_in_the_upper_mid_band) {
    const auto data = sine(8192, 1000.0, 48000.0, 1.0f);
    const auto energies = octaveBandEnergies(data.data(), 8192, 48000.0, 1024, 256);
    CHECK(dominantBand(energies) == 5u);  // UpperMid 720-1440 Hz
}

TEST_CASE(sixty_hz_tone_lands_in_the_low_band) {
    const auto data = sine(8192, 60.0, 48000.0, 1.0f);
    const auto energies = octaveBandEnergies(data.data(), 8192, 48000.0, 1024, 256);
    CHECK(dominantBand(energies) == 1u);  // Low 45-90 Hz
}

TEST_CASE(bass_heavy_signal_is_darker_in_the_highs) {
    const auto data = bassHeavy(8192, 48000.0, 1.0f);
    const auto energies = octaveBandEnergies(data.data(), 8192, 48000.0, 1024, 256);
    CHECK(energies[1] > energies[8]);
}
