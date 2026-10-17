#pragma once

#include "masterizer/domain/dsp/TransientShaper.h"

namespace masterizer::domain::dsp {

/// Stateful version of `applyTransientShaper` for block-based processing.
class TransientProcessor {
public:
    TransientProcessor(double sampleRate, const TransientShaperSettings& settings);
    void process(float* samples, int count);

private:
    double fastCoefficient_ = 0.0;
    double slowCoefficient_ = 0.0;
    double attackGain_ = 1.0;
    double sustainGain_ = 1.0;
    double fast_ = 0.0;
    double slow_ = 0.0;
    bool valid_ = false;
};

}  // namespace masterizer::domain::dsp
