#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"
#include "masterizer/domain/dsp/TapeStage.h"

namespace masterizer::application {

/// Applies the tape stage to every channel of the buffer.
void applyTapeToBuffer(domain::audio::AudioBuffer& buffer,
                       const domain::dsp::TapeSettings& settings);

}  // namespace masterizer::application
