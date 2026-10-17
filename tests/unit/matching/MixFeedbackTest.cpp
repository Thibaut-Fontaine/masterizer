#include "masterizer/domain/matching/MixAdvisor.h"

#include "support/Checks.h"
#include "support/Profiles.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::domain::analysis::AnalysisProfile;
using masterizer::domain::matching::AdvisorRuleset;
using masterizer::domain::matching::adviseFeedback;
using masterizer::domain::matching::MixSuggestion;
using masterizer::test::profileWithBands;

TEST_CASE(feedback_is_stamped_with_the_ruleset_version) {
    const std::vector<double> flat(10, 0.1);
    AnalysisProfile source = profileWithBands(flat);
    AnalysisProfile reference = profileWithBands(flat);
    source.envelope.crestDb = 3.0;
    reference.envelope.crestDb = 12.0;
    AdvisorRuleset rules;
    rules.version = 42;
    const auto feedback = adviseFeedback(source, reference, rules);
    CHECK(feedback.rulesetVersion == 42);
    CHECK(!feedback.suggestions.empty());
    for (const MixSuggestion& suggestion : feedback.suggestions) {
        CHECK(suggestion.rulesetVersion == 42);
    }
}
