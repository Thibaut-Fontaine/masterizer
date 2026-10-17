#include "masterizer/domain/matching/MixAdvisor.h"

#include <vector>

namespace masterizer::domain::matching {
namespace {

using RuleFunction = bool (*)(const RuleContext&, MixSuggestion&);

std::vector<RuleFunction> defaultRules() {
    return {ruleLowEndPunch,        ruleLowEndWidth,        ruleSourceOverCompression,
            ruleLowMidBuildup,      ruleMissingAir,         ruleHarshPresence,
            ruleNarrowImage,        ruleDullHighTransients, ruleResonantBass,
            ruleKickBassConflict,   rulePhaseProblem};
}

}  // namespace

std::vector<MixSuggestion> advise(const analysis::AnalysisProfile& source,
                                  const analysis::AnalysisProfile& reference,
                                  const AdvisorRuleset& rules) {
    return adviseFeedback(source, reference, rules).suggestions;
}

MixFeedback adviseFeedback(const analysis::AnalysisProfile& source,
                           const analysis::AnalysisProfile& reference,
                           const AdvisorRuleset& rules) {
    const ProfileDifference difference = compareProfiles(source, reference);
    const RuleContext context{source, reference, difference, rules};
    MixFeedback feedback;
    feedback.rulesetVersion = rules.version;
    for (RuleFunction rule : defaultRules()) {
        MixSuggestion suggestion;
        if (rule(context, suggestion)) {
            suggestion.rulesetVersion = rules.version;
            feedback.suggestions.push_back(suggestion);
        }
    }
    return feedback;
}

}  // namespace masterizer::domain::matching
