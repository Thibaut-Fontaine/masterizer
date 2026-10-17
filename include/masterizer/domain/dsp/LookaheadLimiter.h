#pragma once

namespace masterizer::domain::dsp {

/// Anticipatory peak limiter. The gain is computed from a short look-ahead
/// window, so peaks are attenuated before they arrive (no hard clipping and
/// less distortion than a zero-look-ahead design).
struct LookaheadLimiterSettings {
    double ceilingDb = -1.0;
    double lookaheadSeconds = 0.005;
    double releaseSeconds = 0.1;
};

void applyLookaheadLimiter(float* samples, int count, double sampleRate,
                           const LookaheadLimiterSettings& settings);

}  // namespace masterizer::domain::dsp
