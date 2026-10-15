#include "masterizer/domain/dsp/TransientShaper.h"

#include "masterizer/domain/dsp/Gain.h"

#include <algorithm>
#include <cmath>

namespace masterizer::domain::dsp {
namespace {

double envelopeStep(double current, double target, double coefficient) {
    return coefficient * current + (1.0 - coefficient) * target;
}

}  // namespace

void applyTransientShaper(float* samples, int count, double sampleRate,
                          const TransientShaperSettings& settings) {
    if (samples == nullptr || count <= 0 || sampleRate <= 0.0) {
        return;
    }
    const double fastCoefficient =
        std::exp(-1.0 / (std::max(settings.fastSeconds, 1e-6) * sampleRate));
    const double slowCoefficient =
        std::exp(-1.0 / (std::max(settings.slowSeconds, 1e-6) * sampleRate));
    const double attackGain = dbToLinear(settings.attackGainDb);
    const double sustainGain = dbToLinear(settings.sustainGainDb);
    double fast = 0.0;
    double slow = 0.0;
    for (int i = 0; i < count; ++i) {
        const double magnitude = std::fabs(static_cast<double>(samples[i]));
        fast = envelopeStep(fast, magnitude, fastCoefficient);
        slow = envelopeStep(slow, magnitude, slowCoefficient);
        const double weight =
            slow > 1e-9 ? std::clamp((fast - slow) / slow, 0.0, 1.0) : 0.0;
        const double gain = (1.0 - weight) * sustainGain + weight * attackGain;
        samples[i] = static_cast<float>(static_cast<double>(samples[i]) * gain);
    }
}

}  // namespace masterizer::domain::dsp
