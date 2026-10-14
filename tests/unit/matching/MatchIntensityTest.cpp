#include "masterizer/domain/matching/MatchIntensity.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::matching::CorrectionBounds;
using masterizer::domain::matching::scaleEqAmount;

namespace {

const CorrectionBounds kBounds{6.0, 6.0, 0.5, 0.2};

}  // namespace

TEST_CASE(zero_intensity_is_neutral) {
    CHECK_NEAR(scaleEqAmount(-4.0, 0.0, kBounds), 0.0, 1e-12);
    CHECK_NEAR(scaleEqAmount(4.0, 0.0, kBounds), 0.0, 1e-12);
}

TEST_CASE(full_intensity_reproduces_the_proposal) {
    CHECK_NEAR(scaleEqAmount(-4.0, 1.0, kBounds), -4.0, 1e-12);
    CHECK_NEAR(scaleEqAmount(3.0, 1.0, kBounds), 3.0, 1e-12);
}

TEST_CASE(above_full_intensity_strengthens_within_bounds) {
    CHECK_NEAR(scaleEqAmount(-2.0, 2.0, kBounds), -4.0, 1e-12);
    CHECK_NEAR(scaleEqAmount(-5.0, 2.0, kBounds), -6.0, 1e-12);  // clamped to max cut
}
