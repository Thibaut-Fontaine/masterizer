#include "masterizer/domain/analysis/BandDynamics.h"

#include "support/Checks.h"
#include "support/Signals.h"
#include "support/SignalsProgram.h"
#include "support/TestRegistry.h"

#include <cstddef>
#include <vector>

using masterizer::domain::analysis::bandVariation;
using masterizer::test::silence;
using masterizer::test::sine;
using masterizer::test::transientRich;

TEST_CASE(band_variation_of_silence_is_zero) {
    const auto data = silence(48000);
    const auto variation = bandVariation(data.data(), 48000, 48000.0, 1024, 256);
    CHECK(variation.size() == 10u);
    for (double value : variation) {
        CHECK_NEAR(value, 0.0, 1e-12);
    }
}

TEST_CASE(steady_tone_has_low_variation) {
    const auto data = sine(48000, 1000.0, 48000.0, 1.0f);
    const auto variation = bandVariation(data.data(), 48000, 48000.0, 1024, 256);
    CHECK(variation[5] < 0.1);  // UpperMid is steady
}

TEST_CASE(transient_material_varies_more_in_the_low_band) {
    const auto steady = sine(48000, 60.0, 48000.0, 1.0f);
    const auto transient = transientRich(48000, 4000, 0.6, 1.0f);
    const auto steadyVar = bandVariation(steady.data(), 48000, 48000.0, 1024, 256);
    const auto transientVar = bandVariation(transient.data(), 48000, 48000.0, 1024, 256);
    CHECK(transientVar[1] > steadyVar[1]);
    CHECK(transientVar[1] > 0.5);
}
