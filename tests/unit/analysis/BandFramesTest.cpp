#include "masterizer/domain/analysis/BandFrames.h"

#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <cstddef>

using masterizer::domain::analysis::frameBandEnergies;
using masterizer::test::silence;
using masterizer::test::sine;

TEST_CASE(frame_band_energies_of_silence_are_zero) {
    const auto data = silence(4096);
    const auto frames = frameBandEnergies(data.data(), 4096, 48000.0, 1024, 256);
    CHECK(!frames.empty());
    for (const auto& frame : frames) {
        for (double energy : frame) {
            CHECK_NEAR(energy, 0.0, 1e-12);
        }
    }
}

TEST_CASE(frame_band_energies_locate_a_tone) {
    const auto data = sine(4096, 1000.0, 48000.0, 1.0f);
    const auto frames = frameBandEnergies(data.data(), 4096, 48000.0, 1024, 256);
    CHECK(!frames.empty());
    for (const auto& frame : frames) {
        CHECK(frame[5] > 0.0);  // UpperMid
    }
}
