#pragma once

#include <vector>

namespace masterizer::domain::audio {

/// Planar multi-channel audio: one float vector per channel.
struct AudioBuffer {
    std::vector<std::vector<float>> channels;
    double sampleRate = 0.0;
};

}  // namespace masterizer::domain::audio
