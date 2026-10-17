#pragma once

#include "masterizer/domain/dsp/Biquad.h"
#include "masterizer/domain/dsp/MultibandCompressor.h"

#include <vector>

namespace masterizer::domain::dsp {

/// Stateful version of `applyMultibandCompressor` for block-based processing.
/// Call `prepare(maxBlockSize)` before processing; larger blocks are split
/// internally, so the scratch buffers never grow in the audio callback.
class MultibandProcessor {
public:
    MultibandProcessor(double sampleRate, const MultibandCompressorSettings& settings);
    void prepare(int maxBlockSize);
    void process(float* samples, int count);

private:
    void processChunk(float* samples, int count);
    MultibandCompressorSettings settings_;
    double sampleRate_ = 0.0;
    BiquadCoefficients lowPass_;
    BiquadCoefficients highPass_;
    BiquadState lowStage1_;
    BiquadState lowStage2_;
    BiquadState highStage1_;
    BiquadState highStage2_;
    CompressorState lowState_;
    CompressorState highState_;
    std::vector<float> low_;
    std::vector<float> high_;
    bool valid_ = false;
};

}  // namespace masterizer::domain::dsp
