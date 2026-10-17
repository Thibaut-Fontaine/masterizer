#include "masterizer/domain/dsp/LookaheadProcessor.h"

#include "masterizer/domain/dsp/Gain.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace masterizer::domain::dsp {

LookaheadProcessor::LookaheadProcessor(double sampleRate,
                                       const LookaheadLimiterSettings& settings) {
    valid_ = sampleRate > 0.0;
    if (!valid_) {
        return;
    }
    lookahead_ = std::max(1, static_cast<int>(std::lround(settings.lookaheadSeconds * sampleRate)));
    ceiling_ = dbToLinear(settings.ceilingDb);
    releaseStep_ =
        1.0 - std::exp(-1.0 / (std::max(settings.releaseSeconds, 1e-6) * sampleRate));
    delay_.assign(static_cast<std::size_t>(lookahead_), 0.0f);
    magnitudes_.assign(static_cast<std::size_t>(lookahead_) + 1u, 0.0f);
    window_.assign(static_cast<std::size_t>(lookahead_) + 2u, 0);
}

int LookaheadProcessor::latencySamples() const {
    return valid_ ? lookahead_ : 0;
}

void LookaheadProcessor::process(float* samples, int count) {
    if (!valid_ || samples == nullptr || count <= 0) {
        return;
    }
    const int capacity = lookahead_ + 2;
    const int magnitudeCapacity = lookahead_ + 1;
    for (int i = 0; i < count; ++i) {
        const long long index = sampleIndex_++;
        const double magnitude = std::fabs(static_cast<double>(samples[i]));
        magnitudes_[static_cast<std::size_t>(index % magnitudeCapacity)] =
            static_cast<float>(magnitude);
        const long long oldest = index - lookahead_;
        while (windowCount_ > 0 && window_[static_cast<std::size_t>(windowHead_)] < oldest) {
            windowHead_ = (windowHead_ + 1) % capacity;
            --windowCount_;
        }
        while (windowCount_ > 0) {
            const int back = (windowHead_ + windowCount_ - 1) % capacity;
            const long long backIndex = window_[static_cast<std::size_t>(back)];
            const double backMagnitude = static_cast<double>(
                magnitudes_[static_cast<std::size_t>(backIndex % magnitudeCapacity)]);
            if (backMagnitude > magnitude) {
                break;
            }
            --windowCount_;
        }
        window_[static_cast<std::size_t>((windowHead_ + windowCount_) % capacity)] = index;
        ++windowCount_;

        const long long peakIndex = window_[static_cast<std::size_t>(windowHead_)];
        const double peak = static_cast<double>(
            magnitudes_[static_cast<std::size_t>(peakIndex % magnitudeCapacity)]);
        const double target = peak > 0.0 ? std::min(1.0, ceiling_ / peak) : 1.0;
        if (target < gain_) {
            gain_ = target;
        } else {
            gain_ = std::min(target, gain_ + releaseStep_ * (1.0 - gain_));
        }

        const double delayed = static_cast<double>(delay_[static_cast<std::size_t>(delayPosition_)]);
        delay_[static_cast<std::size_t>(delayPosition_)] = samples[i];
        delayPosition_ = (delayPosition_ + 1) % lookahead_;
        samples[i] = static_cast<float>(delayed * gain_);
    }
}

}  // namespace masterizer::domain::dsp
