#include "masterizer/domain/matching/DynamicsPlanner.h"

#include "masterizer/domain/dsp/BpmRelease.h"
#include "support/Checks.h"
#include "support/Profiles.h"
#include "support/TestRegistry.h"

#include <algorithm>
#include <vector>

using masterizer::domain::analysis::AnalysisProfile;
using masterizer::domain::matching::DynamicsPlan;
using masterizer::domain::matching::planDynamics;
using masterizer::test::profileWithBands;

namespace {

AnalysisProfile withVariation(double variation) {
    AnalysisProfile profile = profileWithBands(std::vector<double>(10, 0.1));
    profile.bandVariation.assign(10, variation);
    return profile;
}

}  // namespace

TEST_CASE(identical_dynamics_profiles_need_no_compression) {
    const auto plan = planDynamics(withVariation(0.1), withVariation(0.1), 0.0);
    CHECK(!plan.glueEnabled);
    CHECK(!plan.multibandEnabled);
}

TEST_CASE(a_more_dynamic_source_enables_gentle_glue) {
    const auto plan = planDynamics(withVariation(0.2), withVariation(0.1), 0.0);
    CHECK(plan.glueEnabled);
    CHECK(!plan.multibandEnabled);
    CHECK(plan.glue.ratio >= 1.5 && plan.glue.ratio <= 2.0);
}

TEST_CASE(a_much_more_dynamic_source_enables_multiband) {
    const auto plan = planDynamics(withVariation(0.5), withVariation(0.1), 0.0);
    CHECK(plan.glueEnabled);
    CHECK(plan.multibandEnabled);
}

TEST_CASE(tempo_sets_the_suggested_release) {
    const auto plan = planDynamics(withVariation(0.2), withVariation(0.1), 120.0);
    CHECK_NEAR(plan.suggestedReleaseSeconds, 0.125, 1e-9);
}
