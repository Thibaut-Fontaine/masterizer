#include "masterizer/domain/dsp/KWeighting.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::kWeightingHighPass;
using masterizer::domain::dsp::kWeightingShelf;

TEST_CASE(k_weighting_shelf_matches_bs1770_at_48k) {
    const auto c = kWeightingShelf(48000.0);
    CHECK_NEAR(c.b0, 1.535124859587, 1e-9);
    CHECK_NEAR(c.b1, -2.691696189406, 1e-9);
    CHECK_NEAR(c.b2, 1.198392810853, 1e-9);
    CHECK_NEAR(c.a1, -1.690659293182, 1e-9);
    CHECK_NEAR(c.a2, 0.732480774216, 1e-9);
}

TEST_CASE(k_weighting_high_pass_matches_bs1770_at_48k) {
    const auto c = kWeightingHighPass(48000.0);
    CHECK_NEAR(c.b0, 1.0, 1e-12);
    CHECK_NEAR(c.b1, -2.0, 1e-12);
    CHECK_NEAR(c.b2, 1.0, 1e-12);
    CHECK_NEAR(c.a1, -1.990047454834, 1e-9);
    CHECK_NEAR(c.a2, 0.990072250366, 1e-9);
}
