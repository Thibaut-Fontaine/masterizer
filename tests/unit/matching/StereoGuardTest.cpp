#include "masterizer/domain/matching/StereoGuard.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::matching::stereoMatchAllowed;

TEST_CASE(near_mono_references_do_not_allow_width_matching) {
    CHECK(!stereoMatchAllowed(0.02));
    CHECK(!stereoMatchAllowed(0.0));
}

TEST_CASE(wide_enough_references_allow_width_matching) {
    CHECK(stereoMatchAllowed(0.20));
    CHECK(stereoMatchAllowed(0.05));  // inclusive threshold
}
