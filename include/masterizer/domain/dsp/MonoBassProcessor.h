#pragma once

#include "masterizer/domain/dsp/Biquad.h"
#include "masterizer/domain/dsp/MonoBass.h"

namespace masterizer::domain::dsp {

/// Stateful version of `applyMonoBass` for block-based processing.
class MonoBassProcessor {
public:
    MonoBassProcessor(double sampleRate, const MonoBassSettings& settings);
    void process(float* left, float* right, int count);

private:
    MonoBassSettings settings_;
    BiquadCoefficients highPass_;
    BiquadState firstStage_;
    BiquadState secondStage_;
    bool valid_ = false;
};

}  // namespace masterizer::domain::dsp
