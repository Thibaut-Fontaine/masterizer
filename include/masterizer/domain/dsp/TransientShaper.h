#pragma once

namespace masterizer::domain::dsp {

/// Transient shaper settings. Positive `attackGainDb` emphasises attacks;
/// positive `sustainGainDb` emphasises the sustained body.
struct TransientShaperSettings {
    double attackGainDb = 0.0;
    double sustainGainDb = 0.0;
    double fastSeconds = 0.002;
    double slowSeconds = 0.05;
};

/// Applies a dual-envelope transient shaper in place to one channel.
void applyTransientShaper(float* samples, int count, double sampleRate,
                          const TransientShaperSettings& settings);

}  // namespace masterizer::domain::dsp
