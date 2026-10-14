#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"

#include <vector>

namespace masterizer::test {

/// One-channel buffer with the given samples.
masterizer::domain::audio::AudioBuffer monoBuffer(const std::vector<float>& samples,
                                                  double sampleRate);

/// Two-channel buffer from matching-length channels.
masterizer::domain::audio::AudioBuffer stereoBuffer(const std::vector<float>& left,
                                                    const std::vector<float>& right,
                                                    double sampleRate);

}  // namespace masterizer::test
