#include "masterizer/domain/matching/MixAdvisor.h"

#include <algorithm>

namespace masterizer::domain::matching {

bool ruleLowMidBuildup(const RuleContext& context, MixSuggestion& out) {
    if (energyGapDb(context, 3) < context.rules.tonalDb) {
        return false;
    }
    out = makeSuggestion("R4", SuggestionCategory::Tonal, "bandEnergy",
                         "Gentle source cut around 200-300 Hz, or carve the bass.",
                         "low_mid_buildup");
    out.bands = {3};
    out.plausibleSources = {"bass (likely)", "keys/guitars (possible)",
                            "kick body (possible)"};
    out.confidence = "medium";
    out.masterFixable = true;
    return true;
}

bool ruleMissingAir(const RuleContext& context, MixSuggestion& out) {
    const double gap = std::min(energyGapDb(context, 8), energyGapDb(context, 9));
    if (gap > -context.rules.tonalDb) {
        return false;
    }
    out = makeSuggestion("R5", SuggestionCategory::Tonal, "bandEnergy",
                         "Lift air/presence at source; first check monitoring and any low-pass.",
                         "missing_air");
    out.bands = {8, 9};
    out.plausibleSources = {"dark source/samples (likely)", "low-pass (possible)"};
    out.confidence = "medium";
    out.masterFixable = true;
    return true;
}

bool ruleHarshPresence(const RuleContext& context, MixSuggestion& out) {
    if (energyGapDb(context, 7) < context.rules.tonalDb ||
        densityGap(context, 7) < context.rules.densityGap) {
        return false;
    }
    out = makeSuggestion("R6", SuggestionCategory::Transient, "bandEnergy+transientDensity",
                         "Tame ~3-5 kHz on overheads/lead, soften attack.", "harsh_presence");
    out.bands = {7};
    out.plausibleSources = {"hats/cymbals (possible)", "lead (possible)"};
    out.confidence = "medium";
    out.masterFixable = true;
    return true;
}

bool ruleResonantBass(const RuleContext& context, MixSuggestion& out) {
    for (int band = 0; band <= 2; ++band) {
        if (peakinessOf(context, band) > context.rules.peakiness &&
            persistenceOf(context, band) > context.rules.persistence) {
            out = makeSuggestion("R9", SuggestionCategory::Resonance,
                                 "peakiness+persistence", "Dynamic EQ or notch at source.",
                                 "resonant_bass");
            out.bands = {band};
            out.plausibleSources = {"resonant bass/synth (likely)", "room mode (possible)"};
            out.confidence = "medium";
            out.masterFixable = true;
            return true;
        }
    }
    return false;
}

}  // namespace masterizer::domain::matching
