#include "masterizer/application/TapeStages.h"

#include <cstddef>
#include <vector>

namespace masterizer::application {

void applyTapeToBuffer(domain::audio::AudioBuffer& buffer,
                       const domain::dsp::TapeSettings& settings) {
    for (std::vector<float>& channel : buffer.channels) {
        domain::dsp::applyTapeStage(channel.data(), static_cast<int>(channel.size()),
                                    buffer.sampleRate, settings);
    }
}

}  // namespace masterizer::application
