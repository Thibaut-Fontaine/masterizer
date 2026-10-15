#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"
#include "masterizer/domain/dsp/Compressor.h"
#include "masterizer/domain/dsp/DynamicEq.h"
#include "masterizer/domain/dsp/Limiter.h"
#include "masterizer/domain/dsp/MultibandCompressor.h"
#include "masterizer/domain/dsp/TransientShaper.h"

namespace masterizer::application {

/// Buffer-level dynamics and output stages. All operate in place on every channel.
void applyTransientStage(domain::audio::AudioBuffer& buffer,
                         const domain::dsp::TransientShaperSettings& settings);
void applyGlueStage(domain::audio::AudioBuffer& buffer,
                    const domain::dsp::CompressorSettings& settings);
void applyMultibandStage(domain::audio::AudioBuffer& buffer,
                         const domain::dsp::MultibandCompressorSettings& settings);
void applyDynamicEqStage(domain::audio::AudioBuffer& buffer,
                         const domain::dsp::DynamicEqBandSettings& settings);
void applyOutputStage(domain::audio::AudioBuffer& buffer,
                      const domain::dsp::LimiterSettings& limiter, double outputGainDb);

}  // namespace masterizer::application
