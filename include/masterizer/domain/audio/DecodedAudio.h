#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"

#include <string>

namespace masterizer::domain::audio {

/// Outcome of decoding a file into memory.
struct DecodedAudio {
    AudioBuffer buffer;
    std::string path;
    bool succeeded = false;
    std::string error;
};

}  // namespace masterizer::domain::audio
