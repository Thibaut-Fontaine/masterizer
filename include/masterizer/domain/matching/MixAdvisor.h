#pragma once

#include "masterizer/domain/analysis/AnalysisProfile.h"
#include "masterizer/domain/matching/AdvisorRuleset.h"
#include "masterizer/domain/matching/MixSuggestion.h"
#include "masterizer/domain/matching/RuleContext.h"

#include <vector>

namespace masterizer::domain::matching {

/// Produces source-level suggestions from a Source/Reference profile pair.
/// Deterministic, bounded, off-thread, and never part of the render chain.
std::vector<MixSuggestion> advise(const analysis::AnalysisProfile& source,
                                  const analysis::AnalysisProfile& reference,
                                  const AdvisorRuleset& rules);

// Rules R1..R11. Each returns true and fills `out` when it fires.
bool ruleLowEndPunch(const RuleContext& context, MixSuggestion& out);
bool ruleLowEndWidth(const RuleContext& context, MixSuggestion& out);
bool ruleSourceOverCompression(const RuleContext& context, MixSuggestion& out);
bool ruleLowMidBuildup(const RuleContext& context, MixSuggestion& out);
bool ruleMissingAir(const RuleContext& context, MixSuggestion& out);
bool ruleHarshPresence(const RuleContext& context, MixSuggestion& out);
bool ruleNarrowImage(const RuleContext& context, MixSuggestion& out);
bool ruleDullHighTransients(const RuleContext& context, MixSuggestion& out);
bool ruleResonantBass(const RuleContext& context, MixSuggestion& out);
bool ruleKickBassConflict(const RuleContext& context, MixSuggestion& out);
bool rulePhaseProblem(const RuleContext& context, MixSuggestion& out);

}  // namespace masterizer::domain::matching
