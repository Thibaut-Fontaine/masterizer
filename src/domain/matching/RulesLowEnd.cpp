#include "masterizer/domain/matching/MixAdvisor.h"

namespace masterizer::domain::matching {

bool ruleLowEndPunch(const RuleContext& context, MixSuggestion& out) {
    int worstBand = -1;
    double worstGap = 0.0;
    for (int band = 0; band <= 2; ++band) {
        const double gap = variationGap(context, band);
        if (gap < worstGap) {
            worstGap = gap;
            worstBand = band;
        }
    }
    if (worstBand < 0 || worstGap > -context.rules.variationGap) {
        return false;
    }
    out = makeSuggestion("R1", SuggestionCategory::Dynamics, "bandVariation",
                         "Try +1-2 dB on the kick, slow the kick/track compressor attack, "
                         "or carve the bass under the kick.",
                         "low_end_punch_deficit");
    out.bands = {worstBand};
    out.plausibleSources = {"kick (likely)", "bass masking (possible)"};
    out.confidence = densityGap(context, worstBand) < -context.rules.densityGap ? "high"
                                                                                 : "medium";
    out.masterFixable = false;
    return true;
}

bool ruleLowEndWidth(const RuleContext& context, MixSuggestion& out) {
    if (context.source.stereo.correlation >= context.rules.correlationLow) {
        return false;
    }
    if (context.source.stereo.width - context.reference.stereo.width <= context.rules.widthGap) {
        return false;
    }
    out = makeSuggestion("R2", SuggestionCategory::Stereo, "width",
                         "Mono below ~90-100 Hz; check the bass patch.", "low_end_too_wide");
    out.bands = {0, 1};
    out.plausibleSources = {"wide sub/bass (likely)", "stereo kick (possible)"};
    out.confidence = "high";
    out.masterFixable = true;
    return true;
}

bool ruleKickBassConflict(const RuleContext& context, MixSuggestion& out) {
    const bool lowEnergyHigh = energyGapDb(context, 0) > context.rules.tonalDb ||
                               energyGapDb(context, 1) > context.rules.tonalDb;
    if (!lowEnergyHigh || densityGap(context, 1) > -context.rules.densityGap) {
        return false;
    }
    out = makeSuggestion("R10", SuggestionCategory::Dynamics, "bandEnergy+transientDensity",
                         "Sidechain or carve 60-100 Hz; separate the kick and bass.",
                         "kick_bass_conflict");
    out.bands = {0, 1, 3};
    out.plausibleSources = {"kick + bass (likely)"};
    out.confidence = "medium";
    out.masterFixable = false;
    return true;
}

bool rulePhaseProblem(const RuleContext& context, MixSuggestion& out) {
    if (context.source.stereo.correlation > context.rules.correlationNegative) {
        return false;
    }
    out = makeSuggestion("R11", SuggestionCategory::Stereo, "correlation",
                         "Check phase polarity on that source.", "phase_problem");
    out.plausibleSources = {"polarity/phase issue (likely)"};
    out.confidence = "high";
    out.masterFixable = true;
    return true;
}

}  // namespace masterizer::domain::matching
