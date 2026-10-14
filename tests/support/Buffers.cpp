#include "support/Buffers.h"

namespace masterizer::test {

masterizer::domain::audio::AudioBuffer monoBuffer(const std::vector<float>& samples,
                                                  double sampleRate) {
    masterizer::domain::audio::AudioBuffer buffer;
    buffer.channels.push_back(samples);
    buffer.sampleRate = sampleRate;
    return buffer;
}

masterizer::domain::audio::AudioBuffer stereoBuffer(const std::vector<float>& left,
                                                    const std::vector<float>& right,
                                                    double sampleRate) {
    masterizer::domain::audio::AudioBuffer buffer;
    buffer.channels.push_back(left);
    buffer.channels.push_back(right);
    buffer.sampleRate = sampleRate;
    return buffer;
}

}  // namespace masterizer::test
