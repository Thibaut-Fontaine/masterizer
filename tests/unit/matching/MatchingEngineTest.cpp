#include "masterizer/domain/matching/MatchingEngine.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::domain::matching::CorrectionBounds;
using masterizer::domain::matching::CorrectionDirection;
using masterizer::domain::matching::CorrectionTarget;
using masterizer::domain::matching::MatchingEngine;
using masterizer::domain::matching::ProfileDifference;

TEST_CASE(identical_profiles_produce_no_corrections) {
    const MatchingEngine engine{CorrectionBounds{}};
    CHECK(engine.propose(ProfileDifference{}).empty());
}

TEST_CASE(louder_source_band_proposes_a_cut) {
    const MatchingEngine engine{CorrectionBounds{}};
    ProfileDifference difference;
    difference.bands.push_back({3, 4.0});
    const auto proposals = engine.propose(difference);
    CHECK(proposals.size() == 1u);
    CHECK(proposals[0].target == CorrectionTarget::Eq);
    CHECK(proposals[0].band == 3);
    CHECK(proposals[0].direction == CorrectionDirection::Cut);
    CHECK(proposals[0].amountDb < 0.0);
}

TEST_CASE(quieter_source_band_proposes_a_boost) {
    const MatchingEngine engine{CorrectionBounds{}};
    ProfileDifference difference;
    difference.bands.push_back({5, -3.0});
    const auto proposals = engine.propose(difference);
    CHECK(proposals.size() == 1u);
    CHECK(proposals[0].direction == CorrectionDirection::Boost);
    CHECK(proposals[0].amountDb > 0.0);
}

TEST_CASE(tiny_difference_is_ignored) {
    const MatchingEngine engine{CorrectionBounds{}};
    ProfileDifference difference;
    difference.bands.push_back({3, 0.2});
    CHECK(engine.propose(difference).empty());
}

TEST_CASE(correction_amount_is_bounded) {
    const MatchingEngine engine{CorrectionBounds{}};
    ProfileDifference difference;
    difference.bands.push_back({3, 20.0});
    const auto proposals = engine.propose(difference);
    CHECK(proposals.size() == 1u);
    CHECK_NEAR(proposals[0].amountDb, -6.0, 1e-9);
}
