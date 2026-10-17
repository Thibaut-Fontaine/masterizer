#pragma once

#include "masterizer/domain/dsp/LookaheadLimiter.h"

#include <vector>

namespace masterizer::domain::dsp {

/// Causal look-ahead limiter for block-based processing. The audio is delayed
/// by `latencySamples()` so future peaks can reduce the gain before they arrive.
/// This is the real-time counterpart of the offline gain-anticipation limiter;
/// it reports latency while the offline path stays zero-latency.
class LookaheadProcessor {
public:
    LookaheadProcessor(double sampleRate, const LookaheadLimiterSettings& settings);
    int latencySamples() const;
    void process(float* samples, int count);

private:
    int lookahead_ = 1;
    double ceiling_ = 1.0;
    double releaseStep_ = 0.0;
    double gain_ = 1.0;
    int delayPosition_ = 0;
    long long sampleIndex_ = 0;
    std::vector<float> delay_;
    std::vector<float> magnitudes_;
    std::vector<long long> window_;
    int windowHead_ = 0;
    int windowCount_ = 0;
    bool valid_ = false;
};

}  // namespace masterizer::domain::dsp
