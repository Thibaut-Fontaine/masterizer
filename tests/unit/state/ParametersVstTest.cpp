#include "masterizer/domain/state/Parameters.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <algorithm>
#include <initializer_list>
#include <vector>

using masterizer::domain::state::allParameters;
using masterizer::domain::state::clampParameter;
using masterizer::domain::state::ParameterId;
using masterizer::domain::state::specOf;

TEST_CASE(master_profile_is_a_discrete_soft_loud_switch) {
    const auto spec = specOf(ParameterId::MasterProfile);
    CHECK_NEAR(spec.minValue, 0.0, 1e-12);
    CHECK_NEAR(spec.maxValue, 1.0, 1e-12);
    CHECK_NEAR(spec.defaultValue, 0.0, 1e-12);  // 0 = soft, 1 = loud
    CHECK_NEAR(clampParameter(ParameterId::MasterProfile, 5.0), 1.0, 1e-12);
}

TEST_CASE(vst_module_toggles_have_safe_defaults) {
    CHECK_NEAR(specOf(ParameterId::FineEqEnabled).defaultValue, 0.0, 1e-12);
    CHECK_NEAR(specOf(ParameterId::TapeEnabled).defaultValue, 1.0, 1e-12);
    CHECK_NEAR(specOf(ParameterId::LimiterEnabled).defaultValue, 1.0, 1e-12);
    CHECK_NEAR(specOf(ParameterId::CrestGuardEnabled).defaultValue, 1.0, 1e-12);
}

TEST_CASE(new_vst_parameters_are_in_the_stable_list) {
    const std::vector<ParameterId> parameters = allParameters();
    for (ParameterId id : {ParameterId::MasterProfile, ParameterId::FineEqEnabled,
                           ParameterId::TapeEnabled, ParameterId::LimiterEnabled,
                           ParameterId::CrestGuardEnabled}) {
        CHECK(std::find(parameters.begin(), parameters.end(), id) != parameters.end());
    }
}
