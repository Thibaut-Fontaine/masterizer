#include "masterizer/domain/dsp/Gain.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <cmath>
#include <limits>

using masterizer::domain::dsp::clampGainDb;
using masterizer::domain::dsp::dbToLinear;
using masterizer::domain::dsp::GainBounds;
using masterizer::domain::dsp::linearToDb;
using masterizer::domain::dsp::withinGainBounds;

namespace {

constexpr GainBounds kSixDb{-6.0, 6.0};

}  // namespace

TEST_CASE(db_to_linear_known_values) {
    CHECK_NEAR(dbToLinear(0.0), 1.0, 1e-12);
    CHECK_NEAR(dbToLinear(6.0206), 2.0, 1e-4);
    CHECK_NEAR(dbToLinear(-6.0206), 0.5, 1e-4);
}

TEST_CASE(linear_to_db_inverts_and_handles_zero) {
    CHECK_NEAR(linearToDb(1.0), 0.0, 1e-12);
    CHECK_NEAR(linearToDb(2.0), 6.0206, 1e-4);
    CHECK(std::isinf(linearToDb(0.0)) && linearToDb(0.0) < 0.0);
}

TEST_CASE(clamp_gain_respects_bounds) {
    CHECK_NEAR(clampGainDb(-10.0, kSixDb), -6.0, 1e-12);
    CHECK_NEAR(clampGainDb(10.0, kSixDb), 6.0, 1e-12);
    CHECK_NEAR(clampGainDb(3.0, kSixDb), 3.0, 1e-12);
}

TEST_CASE(within_gain_bounds_rejects_nan_and_outliers) {
    CHECK(withinGainBounds(0.0, kSixDb));
    CHECK(!withinGainBounds(7.0, kSixDb));
    const double nan = std::numeric_limits<double>::quiet_NaN();
    CHECK(!withinGainBounds(nan, kSixDb));
}
