#pragma once

#include "masterizer/domain/dsp/Biquad.h"
#include "masterizer/domain/dsp/DynamicEq.h"

namespace masterizer::domain::dsp {

/// Stateful version of `applyDynamicEqBand` for block-based processing.
class DynamicEqProcessor {
public:
    DynamicEqProcessor(double sampleRate, const DynamicEqBandSettings& settings);
    void process(float* samples, int count);

private:
    DynamicEqBandSettings settings_;
    BiquadCoefficients band_;
    BiquadState state_;
    double attack_ = 0.0;
    double release_ = 0.0;
    double envelope_ = 0.0;
    bool valid_ = false;
};

}  // namespace masterizer::domain::dsp
