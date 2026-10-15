#include "masterizer/application/DynamicsStages.h"

#include "masterizer/domain/dsp/GainStage.h"

#include <cstddef>
#include <vector>

namespace masterizer::application {

void applyTransientStage(domain::audio::AudioBuffer& buffer,
                         const domain::dsp::TransientShaperSettings& settings) {
    for (std::vector<float>& channel : buffer.channels) {
        domain::dsp::applyTransientShaper(channel.data(), static_cast<int>(channel.size()),
                                          buffer.sampleRate, settings);
    }
}

void applyGlueStage(domain::audio::AudioBuffer& buffer,
                    const domain::dsp::CompressorSettings& settings) {
    for (std::vector<float>& channel : buffer.channels) {
        domain::dsp::applyCompressor(channel.data(), static_cast<int>(channel.size()),
                                     buffer.sampleRate, settings);
    }
}

void applyMultibandStage(domain::audio::AudioBuffer& buffer,
                         const domain::dsp::MultibandCompressorSettings& settings) {
    for (std::vector<float>& channel : buffer.channels) {
        domain::dsp::applyMultibandCompressor(channel.data(), static_cast<int>(channel.size()),
                                              buffer.sampleRate, settings);
    }
}

void applyDynamicEqStage(domain::audio::AudioBuffer& buffer,
                         const domain::dsp::DynamicEqBandSettings& settings) {
    for (std::vector<float>& channel : buffer.channels) {
        domain::dsp::applyDynamicEqBand(channel.data(), static_cast<int>(channel.size()),
                                        buffer.sampleRate, settings);
    }
}

void applyOutputStage(domain::audio::AudioBuffer& buffer,
                      const domain::dsp::LimiterSettings& limiter, double outputGainDb) {
    for (std::vector<float>& channel : buffer.channels) {
        const int frames = static_cast<int>(channel.size());
        domain::dsp::applyLimiter(channel.data(), frames, buffer.sampleRate, limiter);
        if (outputGainDb != 0.0) {
            domain::dsp::applyGain(channel.data(), frames, outputGainDb);
        }
    }
}

}  // namespace masterizer::application
