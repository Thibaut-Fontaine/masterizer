#include "masterizer/domain/matching/MixAdvisor.h"

namespace masterizer::domain::matching {

bool ruleNarrowImage(const RuleContext& context, MixSuggestion& out) {
    if (context.reference.stereo.width - context.source.stereo.width < context.rules.widthGap) {
        return false;
    }
    out = makeSuggestion("R7", SuggestionCategory::Stereo, "width",
                         "Widen pads/keys/reverb; check the stereo bus.", "narrow_image");
    out.plausibleSources = {"over-monoed mix (likely)"};
    out.confidence = "medium";
    out.masterFixable = true;
    return true;
}

}  // namespace masterizer::domain::matching
