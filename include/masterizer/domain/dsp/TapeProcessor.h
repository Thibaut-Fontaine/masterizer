#pragma once

#include "masterizer/domain/dsp/Biquad.h"
#include "masterizer/domain/dsp/TapeStage.h"

namespace masterizer::domain::dsp {

/// Stateful version of `applyTapeStage` for block-based processing.
class TapeProcessor {
public:
    TapeProcessor(double sampleRate, const TapeSettings& settings);
    void process(float* samples, int count);

private:
    BiquadCoefficients lowShelf_;
    BiquadCoefficients highShelf_;
    BiquadState lowState_;
    BiquadState highState_;
    double drive_ = 0.0;
    double compensation_ = 1.0;
    double mix_ = 1.0;
    bool valid_ = false;
};

}  // namespace masterizer::domain::dsp
