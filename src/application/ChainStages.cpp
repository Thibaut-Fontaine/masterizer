#include "masterizer/application/ChainStages.h"

#include "masterizer/domain/dsp/ParametricEq.h"
#include "masterizer/domain/dsp/StereoWidth.h"
#include "masterizer/domain/matching/EqDesigner.h"
#include "masterizer/domain/matching/MatchIntensity.h"

#include <cstddef>

namespace masterizer::application {

void applyMonoBassStage(domain::audio::AudioBuffer& buffer,
                        const domain::dsp::MonoBassSettings& settings) {
    if (buffer.channels.size() < 2u) {
        return;
    }
    domain::dsp::applyMonoBass(buffer.channels[0].data(), buffer.channels[1].data(),
                               static_cast<int>(buffer.channels[0].size()), buffer.sampleRate,
                               settings);
}

void applySaturationStage(domain::audio::AudioBuffer& buffer,
                          const domain::dsp::SaturationSettings& settings) {
    for (std::vector<float>& channel : buffer.channels) {
        domain::dsp::applySaturation(channel.data(), static_cast<int>(channel.size()), settings);
    }
}

void applyResonanceStage(domain::audio::AudioBuffer& buffer,
                         const domain::analysis::AnalysisProfile& source,
                         const domain::matching::ResonanceSettings& settings) {
    std::vector<double> power = source.spectrum;
    for (double& value : power) {
        value *= value;
    }
    const int fftSize = power.empty() ? 0 : 2 * (static_cast<int>(power.size()) - 1);
    const std::vector<domain::dsp::EqBand> bands =
        domain::matching::detectResonances(power, buffer.sampleRate, fftSize, settings);
    for (std::vector<float>& channel : buffer.channels) {
        domain::dsp::applyEqBands(channel.data(), static_cast<int>(channel.size()),
                                  buffer.sampleRate, bands);
    }
}

void applyEqStage(domain::audio::AudioBuffer& buffer,
                  const std::vector<domain::matching::CorrectionProposal>& proposals,
                  double intensity, const domain::matching::CorrectionBounds& bounds) {
    std::vector<domain::dsp::EqBand> bands = domain::matching::designEq(proposals);
    for (domain::dsp::EqBand& band : bands) {
        band.gainDb =
            domain::matching::scaleEqAmount(band.gainDb, intensity, bounds);
    }
    for (std::vector<float>& channel : buffer.channels) {
        domain::dsp::applyEqBands(channel.data(), static_cast<int>(channel.size()),
                                  buffer.sampleRate, bands);
    }
}

void applyStereoStage(domain::audio::AudioBuffer& buffer, double widthScale) {
    if (buffer.channels.size() < 2u) {
        return;
    }
    domain::dsp::applyStereoWidth(buffer.channels[0].data(), buffer.channels[1].data(),
                                  static_cast<int>(buffer.channels[0].size()), widthScale);
}

}  // namespace masterizer::application
