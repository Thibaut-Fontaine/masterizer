#include "masterizer/domain/dsp/TransientProcessor.h"

#include "masterizer/domain/dsp/Gain.h"

#include <algorithm>
#include <cmath>

namespace masterizer::domain::dsp {
namespace {

double envelopeStep(double current, double target, double coefficient) {
    return coefficient * current + (1.0 - coefficient) * target;
}

}  // namespace

TransientProcessor::TransientProcessor(double sampleRate,
                                       const TransientShaperSettings& settings) {
    valid_ = sampleRate > 0.0;
    if (!valid_) {
        return;
    }
    fastCoefficient_ =
        std::exp(-1.0 / (std::max(settings.fastSeconds, 1e-6) * sampleRate));
    slowCoefficient_ =
        std::exp(-1.0 / (std::max(settings.slowSeconds, 1e-6) * sampleRate));
    attackGain_ = dbToLinear(settings.attackGainDb);
    sustainGain_ = dbToLinear(settings.sustainGainDb);
}

void TransientProcessor::process(float* samples, int count) {
    if (!valid_ || samples == nullptr || count <= 0) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        const double magnitude = std::fabs(static_cast<double>(samples[i]));
        fast_ = envelopeStep(fast_, magnitude, fastCoefficient_);
        slow_ = envelopeStep(slow_, magnitude, slowCoefficient_);
        const double weight = slow_ > 1e-9 ? std::clamp((fast_ - slow_) / slow_, 0.0, 1.0) : 0.0;
        const double gain = (1.0 - weight) * sustainGain_ + weight * attackGain_;
        samples[i] = static_cast<float>(static_cast<double>(samples[i]) * gain);
    }
}

}  // namespace masterizer::domain::dsp
