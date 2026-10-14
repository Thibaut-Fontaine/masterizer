#include "masterizer/domain/audio/AudioBufferOps.h"

namespace masterizer::domain::audio {

int channelCount(const AudioBuffer& buffer) {
    return static_cast<int>(buffer.channels.size());
}

int frameCount(const AudioBuffer& buffer) {
    return buffer.channels.empty() ? 0 : static_cast<int>(buffer.channels.front().size());
}

}  // namespace masterizer::domain::audio
