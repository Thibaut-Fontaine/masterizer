#pragma once

#include "masterizer/domain/analysis/AnalysisProfile.h"
#include "masterizer/domain/matching/AdvisorRuleset.h"
#include "masterizer/domain/matching/MixSuggestion.h"
#include "masterizer/domain/matching/ProfileDifference.h"

namespace masterizer::domain::matching {

/// Everything a Mix Advisor rule needs. All accessors are bounds-safe and
/// return 0 for missing bands.
struct RuleContext {
    const analysis::AnalysisProfile& source;
    const analysis::AnalysisProfile& reference;
    const ProfileDifference& difference;
    const AdvisorRuleset& rules;
};

double energyGapDb(const RuleContext& context, int band);
double variationGap(const RuleContext& context, int band);
double densityGap(const RuleContext& context, int band);
double peakinessOf(const RuleContext& context, int band);
double persistenceOf(const RuleContext& context, int band);

/// Small helper to fill the common suggestion fields.
inline MixSuggestion makeSuggestion(const char* id, SuggestionCategory category,
                                    const char* feature, const char* action,
                                    const char* reasonCode) {
    MixSuggestion suggestion;
    suggestion.id = id;
    suggestion.category = category;
    suggestion.feature = feature;
    suggestion.action = action;
    suggestion.reasonCode = reasonCode;
    return suggestion;
}

}  // namespace masterizer::domain::matching
