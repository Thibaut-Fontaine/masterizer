#include "support/Stereo.h"

#include "masterizer/domain/dsp/Correlation.h"
#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::correlation;
using masterizer::test::antiPhase;
using masterizer::test::wideStereo;

TEST_CASE(wide_stereo_channels_are_uncorrelated) {
    const auto pair = wideStereo(48000, 42u, 0.5f);
    CHECK(pair.left.size() == 48000u);
    CHECK(pair.right.size() == 48000u);
    const double corr = correlation(pair.left.data(), pair.right.data(), 48000);
    CHECK(corr > -0.1 && corr < 0.1);
}

TEST_CASE(anti_phase_channels_are_fully_negatively_correlated) {
    const auto pair = antiPhase(48000, 440.0, 48000.0, 0.8f);
    const double corr = correlation(pair.left.data(), pair.right.data(), 48000);
    CHECK_NEAR(corr, -1.0, 1e-9);
}
