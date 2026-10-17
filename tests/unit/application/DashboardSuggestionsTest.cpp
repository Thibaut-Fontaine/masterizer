#include "masterizer/application/DashboardModel.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::application::buildDashboard;
using masterizer::application::DashboardInput;
using masterizer::application::DashboardModel;
using masterizer::domain::matching::MixSuggestion;

TEST_CASE(mix_suggestions_pass_through_the_dashboard) {
    DashboardInput input;
    MixSuggestion suggestion;
    suggestion.id = "R1";
    input.suggestions.push_back(suggestion);
    const DashboardModel model = buildDashboard(input);
    CHECK(model.suggestions.size() == 1u);
    CHECK(model.suggestions[0].id == "R1");
}
