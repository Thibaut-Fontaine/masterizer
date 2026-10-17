#include "masterizer/domain/dsp/MultibandProcessor.h"

#include "masterizer/domain/dsp/FilterDesign.h"

#include <algorithm>
#include <cstddef>

namespace masterizer::domain::dsp {
namespace {

constexpr double kButterworthQ = 0.7071067811865476;
constexpr int kDefaultBlock = 2048;

}  // namespace

MultibandProcessor::MultibandProcessor(double sampleRate,
                                       const MultibandCompressorSettings& settings)
    : settings_(settings), sampleRate_(sampleRate) {
    valid_ = sampleRate > 0.0;
    if (!valid_) {
        return;
    }
    lowPass_ = lowpassCoefficients(sampleRate, settings.crossoverHz, kButterworthQ);
    highPass_ = highpassCoefficients(sampleRate, settings.crossoverHz, kButterworthQ);
    prepare(kDefaultBlock);
}

void MultibandProcessor::prepare(int maxBlockSize) {
    const int size = std::max(1, maxBlockSize);
    low_.assign(static_cast<std::size_t>(size), 0.0f);
    high_.assign(static_cast<std::size_t>(size), 0.0f);
}

void MultibandProcessor::process(float* samples, int count) {
    if (!valid_ || samples == nullptr || count <= 0) {
        return;
    }
    int offset = 0;
    const int chunk = static_cast<int>(low_.size());
    while (offset < count) {
        const int size = std::min(chunk, count - offset);
        processChunk(samples + offset, size);
        offset += size;
    }
}

void MultibandProcessor::processChunk(float* samples, int count) {
    for (int i = 0; i < count; ++i) {
        const double value = static_cast<double>(samples[i]);
        double lowValue = processBiquad(lowPass_, lowStage1_, value);
        lowValue = processBiquad(lowPass_, lowStage2_, lowValue);
        double highValue = processBiquad(highPass_, highStage1_, value);
        highValue = processBiquad(highPass_, highStage2_, highValue);
        low_[static_cast<std::size_t>(i)] = static_cast<float>(lowValue);
        high_[static_cast<std::size_t>(i)] = static_cast<float>(highValue);
    }
    const int frames = count;
    for (int i = 0; i < frames; ++i) {
        low_[static_cast<std::size_t>(i)] = static_cast<float>(
            compressSample(static_cast<double>(low_[static_cast<std::size_t>(i)]), settings_.low,
                           sampleRate_, lowState_));
    }
    for (int i = 0; i < frames; ++i) {
        high_[static_cast<std::size_t>(i)] = static_cast<float>(
            compressSample(static_cast<double>(high_[static_cast<std::size_t>(i)]), settings_.high,
                           sampleRate_, highState_));
    }
    for (int i = 0; i < frames; ++i) {
        samples[i] = low_[static_cast<std::size_t>(i)] + high_[static_cast<std::size_t>(i)];
    }
}

}  // namespace masterizer::domain::dsp
