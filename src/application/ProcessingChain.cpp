#include "masterizer/application/ProcessingChain.h"

#include "masterizer/application/ChainStages.h"
#include "masterizer/application/DynamicsStages.h"
#include "masterizer/domain/matching/MatchingEngine.h"
#include "masterizer/domain/matching/ProfileDifference.h"

namespace masterizer::application {

ChainResult process(const domain::audio::AudioBuffer& input,
                    const domain::analysis::AnalysisProfile& source,
                    const domain::analysis::AnalysisProfile& reference,
                    const ChainSettings& settings) {
    ChainResult result;
    result.buffer = input;

    const domain::matching::ProfileDifference difference =
        domain::matching::compareProfiles(source, reference);
    const domain::matching::MatchingEngine engine{settings.bounds};
    result.proposals = engine.propose(difference);

    if (settings.monoBassEnabled) {
        applyMonoBassStage(result.buffer, settings.monoBass);
    }
    if (settings.saturationEnabled) {
        applySaturationStage(result.buffer, settings.saturation);
    }
    if (settings.resonanceEnabled) {
        applyResonanceStage(result.buffer, source, settings.resonance);
    }
    if (settings.eqEnabled) {
        applyEqStage(result.buffer, result.proposals, settings.matchIntensity, settings.bounds);
    }
    if (settings.stereoEnabled) {
        applyStereoStage(result.buffer, settings.stereoWidthScale);
    }
    if (settings.transientEnabled) {
        applyTransientStage(result.buffer, settings.transient);
    }
    if (settings.glueEnabled) {
        applyGlueStage(result.buffer, settings.glue);
    }
    if (settings.multibandEnabled) {
        applyMultibandStage(result.buffer, settings.multiband);
    }
    if (settings.dynamicEqEnabled) {
        applyDynamicEqStage(result.buffer, settings.dynamicEq);
    }
    if (settings.limiterEnabled) {
        applyOutputStage(result.buffer, settings.limiter, settings.outputGainDb);
    }
    return result;
}

}  // namespace masterizer::application
