#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"

namespace masterizer::domain::audio {

/// Number of channels (0 for an empty buffer).
int channelCount(const AudioBuffer& buffer);

/// Number of frames in the first channel (0 for an empty buffer).
int frameCount(const AudioBuffer& buffer);

}  // namespace masterizer::domain::audio
