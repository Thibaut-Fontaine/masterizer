#include "masterizer/domain/matching/MixAdvisor.h"

#include <algorithm>

namespace masterizer::domain::matching {

bool ruleSourceOverCompression(const RuleContext& context, MixSuggestion& out) {
    const double gap = context.reference.envelope.crestDb - context.source.envelope.crestDb;
    if (gap < context.rules.crestGapDb) {
        return false;
    }
    out = makeSuggestion("R3", SuggestionCategory::Dynamics, "crestFactor",
                         "Back off the mix-bus limiter, keep 1-2 dB more headroom, re-bounce; "
                         "a master cannot recreate transients.",
                         "source_over_compression");
    out.plausibleSources = {"mix-bus limiter/compressor (likely)"};
    out.confidence = "high";
    out.masterFixable = false;
    return true;
}

bool ruleDullHighTransients(const RuleContext& context, MixSuggestion& out) {
    const double gap = std::min(densityGap(context, 7), densityGap(context, 8));
    if (gap > -context.rules.densityGap) {
        return false;
    }
    out = makeSuggestion("R8", SuggestionCategory::Transient, "transientDensity",
                         "Reduce limiting/compression on the highs, restore transient snap.",
                         "dull_high_transients");
    out.bands = {7, 8};
    out.plausibleSources = {"over-limited cymbals (likely)",
                            "over-compressed overheads (possible)"};
    out.confidence = "medium";
    out.masterFixable = false;
    return true;
}

}  // namespace masterizer::domain::matching
