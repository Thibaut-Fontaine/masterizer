#include "masterizer/application/ProcessingChain.h"

#include "masterizer/domain/dsp/ParametricEq.h"
#include "masterizer/domain/matching/EqDesigner.h"
#include "masterizer/domain/matching/MatchIntensity.h"
#include "masterizer/domain/matching/MatchingEngine.h"
#include "masterizer/domain/matching/ProfileDifference.h"

#include <cstddef>

namespace masterizer::application {
namespace {

using domain::audio::AudioBuffer;
using domain::dsp::EqBand;
using domain::matching::CorrectionProposal;

std::vector<EqBand> scaledBands(const std::vector<CorrectionProposal>& proposals,
                                const ChainSettings& settings) {
    std::vector<EqBand> bands = domain::matching::designEq(proposals);
    for (EqBand& band : bands) {
        band.gainDb = domain::matching::scaleEqAmount(band.gainDb, settings.matchIntensity,
                                                      settings.bounds);
    }
    return bands;
}

void applyEqToChannels(AudioBuffer& buffer, const std::vector<EqBand>& bands) {
    for (std::vector<float>& channel : buffer.channels) {
        domain::dsp::applyEqBands(channel.data(), static_cast<int>(channel.size()),
                                  buffer.sampleRate, bands);
    }
}

void applyLimiterToChannels(AudioBuffer& buffer, const domain::dsp::LimiterSettings& settings) {
    for (std::vector<float>& channel : buffer.channels) {
        domain::dsp::applyLimiter(channel.data(), static_cast<int>(channel.size()),
                                  buffer.sampleRate, settings);
    }
}

}  // namespace

ChainResult process(const AudioBuffer& input, const domain::analysis::AnalysisProfile& source,
                    const domain::analysis::AnalysisProfile& reference,
                    const ChainSettings& settings) {
    ChainResult result;
    result.buffer = input;

    const domain::matching::ProfileDifference difference =
        domain::matching::compareProfiles(source, reference);
    const domain::matching::MatchingEngine engine{settings.bounds};
    result.proposals = engine.propose(difference);

    if (settings.eqEnabled) {
        applyEqToChannels(result.buffer, scaledBands(result.proposals, settings));
    }
    if (settings.monoBassEnabled && result.buffer.channels.size() >= 2u) {
        domain::dsp::applyMonoBass(result.buffer.channels[0].data(),
                                   result.buffer.channels[1].data(),
                                   static_cast<int>(result.buffer.channels[0].size()),
                                   result.buffer.sampleRate, settings.monoBass);
    }
    if (settings.limiterEnabled) {
        applyLimiterToChannels(result.buffer, settings.limiter);
    }
    return result;
}

}  // namespace masterizer::application
