#include "masterizer/domain/analysis/BandResonance.h"

#include "support/Checks.h"
#include "support/Signals.h"
#include "support/SignalsProgram.h"
#include "support/TestRegistry.h"

#include <cstddef>

using masterizer::domain::analysis::analyzeBandResonance;
using masterizer::test::silence;
using masterizer::test::sine;
using masterizer::test::transientRich;

TEST_CASE(silence_has_no_resonance) {
    const auto data = silence(48000);
    const auto profile = analyzeBandResonance(data.data(), 48000, 48000.0, 1024, 256);
    CHECK(profile.persistence.size() == 10u);
    for (std::size_t band = 0; band < profile.persistence.size(); ++band) {
        CHECK_NEAR(profile.persistence[band], 0.0, 1e-12);
        CHECK_NEAR(profile.peakiness[band], 0.0, 1e-12);
    }
}

TEST_CASE(a_pure_tone_is_peakier_than_flat_noise) {
    const auto data = sine(48000, 1000.0, 48000.0, 1.0f);
    const auto profile = analyzeBandResonance(data.data(), 48000, 48000.0, 1024, 256);
    CHECK(profile.peakiness[5] > 5.0);  // one dominant bin in UpperMid
}

TEST_CASE(transient_material_is_persistently_above_its_mean) {
    const auto data = transientRich(48000, 4000, 0.6, 1.0f);
    const auto profile = analyzeBandResonance(data.data(), 48000, 48000.0, 1024, 256);
    CHECK(profile.persistence[1] > 0.05);
}
