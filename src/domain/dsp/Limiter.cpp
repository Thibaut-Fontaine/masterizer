#include "masterizer/domain/dsp/Limiter.h"

#include "masterizer/domain/dsp/Gain.h"

#include <algorithm>
#include <cmath>

namespace masterizer::domain::dsp {

void applyLimiter(float* samples, int count, double sampleRate,
                  const LimiterSettings& settings) {
    if (samples == nullptr || count <= 0 || sampleRate <= 0.0 ||
        settings.releaseSeconds <= 0.0) {
        return;
    }
    const double ceiling = dbToLinear(settings.ceilingDb);
    const double releaseStep = 1.0 - std::exp(-1.0 / (settings.releaseSeconds * sampleRate));
    double gain = 1.0;
    for (int i = 0; i < count; ++i) {
        const double magnitude = std::fabs(static_cast<double>(samples[i]));
        const double required = magnitude > 0.0 ? std::min(1.0, ceiling / magnitude) : 1.0;
        if (required < gain) {
            gain = required;  // instant attack
        } else {
            gain += releaseStep * (1.0 - gain);  // smooth release
        }
        samples[i] = static_cast<float>(static_cast<double>(samples[i]) * gain);
    }
}

}  // namespace masterizer::domain::dsp
