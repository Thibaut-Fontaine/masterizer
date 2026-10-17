#pragma once

#include "masterizer/domain/dsp/Compressor.h"

namespace masterizer::domain::dsp {

/// Stateful version of `applyCompressor` for block-based processing.
class CompressorProcessor {
public:
    CompressorProcessor(double sampleRate, const CompressorSettings& settings);
    void process(float* samples, int count);

private:
    CompressorSettings settings_;
    double sampleRate_ = 0.0;
    CompressorState state_;
    bool valid_ = false;
};

}  // namespace masterizer::domain::dsp
