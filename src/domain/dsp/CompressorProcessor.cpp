#include "masterizer/domain/dsp/CompressorProcessor.h"

namespace masterizer::domain::dsp {

CompressorProcessor::CompressorProcessor(double sampleRate, const CompressorSettings& settings)
    : settings_(settings), sampleRate_(sampleRate) {
    valid_ = sampleRate > 0.0;
}

void CompressorProcessor::process(float* samples, int count) {
    if (!valid_ || samples == nullptr || count <= 0) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        samples[i] = static_cast<float>(compressSample(static_cast<double>(samples[i]), settings_,
                                                       sampleRate_, state_));
    }
}

}  // namespace masterizer::domain::dsp
