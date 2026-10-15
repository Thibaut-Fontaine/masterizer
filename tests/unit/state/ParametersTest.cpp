#include "masterizer/domain/state/Parameters.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <set>
#include <string>

using masterizer::domain::state::allParameters;
using masterizer::domain::state::clampParameter;
using masterizer::domain::state::ParameterId;
using masterizer::domain::state::specOf;

TEST_CASE(match_intensity_spans_zero_to_two) {
    const auto spec = specOf(ParameterId::MatchIntensity);
    CHECK_NEAR(spec.minValue, 0.0, 1e-12);
    CHECK_NEAR(spec.maxValue, 2.0, 1e-12);
    CHECK_NEAR(spec.defaultValue, 1.0, 1e-12);
}

TEST_CASE(true_peak_ceiling_defaults_to_minus_one) {
    const auto spec = specOf(ParameterId::TruePeakCeilingDb);
    CHECK_NEAR(spec.defaultValue, -1.0, 1e-12);
    CHECK(spec.maxValue <= 0.0);
}

TEST_CASE(clamping_respects_each_range) {
    CHECK_NEAR(clampParameter(ParameterId::MatchIntensity, 5.0), 2.0, 1e-12);
    CHECK_NEAR(clampParameter(ParameterId::MatchIntensity, -1.0), 0.0, 1e-12);
    CHECK_NEAR(clampParameter(ParameterId::TruePeakCeilingDb, 3.0), 0.0, 1e-12);
}

TEST_CASE(parameter_ids_are_unique_and_non_empty) {
    std::set<std::string> ids;
    for (ParameterId id : allParameters()) {
        const std::string text = specOf(id).id;
        CHECK(!text.empty());
        CHECK(ids.insert(text).second);
    }
}
