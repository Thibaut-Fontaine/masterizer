#include "masterizer/domain/dsp/BpmRelease.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::suggestReleaseSeconds;

TEST_CASE(release_is_a_fraction_of_a_beat) {
    CHECK_NEAR(suggestReleaseSeconds(120.0, 0.25), 0.125, 1e-12);
    CHECK_NEAR(suggestReleaseSeconds(60.0, 1.0), 1.0, 1e-12);
}

TEST_CASE(release_is_clamped_to_a_sane_range) {
    CHECK_NEAR(suggestReleaseSeconds(600.0, 0.01), 0.02, 1e-12);
}

TEST_CASE(invalid_tempo_returns_a_neutral_default) {
    CHECK_NEAR(suggestReleaseSeconds(0.0, 0.25), 0.1, 1e-12);
    CHECK_NEAR(suggestReleaseSeconds(-120.0, 0.25), 0.1, 1e-12);
}
