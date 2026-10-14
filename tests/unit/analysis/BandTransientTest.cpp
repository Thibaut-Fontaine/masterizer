#include "masterizer/domain/analysis/BandTransient.h"

#include "support/Checks.h"
#include "support/Signals.h"
#include "support/SignalsProgram.h"
#include "support/TestRegistry.h"

#include <cstddef>

using masterizer::domain::analysis::analyzeBandTransient;
using masterizer::test::silence;
using masterizer::test::sine;
using masterizer::test::transientRich;

TEST_CASE(silence_has_no_band_transients) {
    const auto data = silence(48000);
    const auto profile = analyzeBandTransient(data.data(), 48000, 48000.0, 1024, 256);
    CHECK(profile.density.size() == 10u);
    for (std::size_t band = 0; band < profile.density.size(); ++band) {
        CHECK_NEAR(profile.density[band], 0.0, 1e-12);
        CHECK_NEAR(profile.crest[band], 0.0, 1e-12);
    }
}

TEST_CASE(steady_tone_has_low_band_crest) {
    const auto data = sine(48000, 1000.0, 48000.0, 1.0f);
    const auto profile = analyzeBandTransient(data.data(), 48000, 48000.0, 1024, 256);
    CHECK(profile.crest[5] < 2.5);
}

TEST_CASE(transient_material_has_low_band_onset_density) {
    const auto data = transientRich(48000, 4000, 0.6, 1.0f);
    const auto profile = analyzeBandTransient(data.data(), 48000, 48000.0, 1024, 256);
    CHECK(profile.density[1] > 0.0);
    CHECK(profile.crest[1] > 2.0);
}
