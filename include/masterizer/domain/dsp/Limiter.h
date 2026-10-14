#pragma once

namespace masterizer::domain::dsp {

/// Peak limiter settings.
struct LimiterSettings {
    double ceilingDb = -1.0;       // default True Peak ceiling
    double releaseSeconds = 0.1;
};

/// In-place peak limiter: instant gain reduction to keep the sample peak at or
/// below the ceiling, with a smooth release. Controls the sampled peak; the
/// resulting True Peak is reported by the OutputValidator.
void applyLimiter(float* samples, int count, double sampleRate,
                  const LimiterSettings& settings);

}  // namespace masterizer::domain::dsp
