#pragma once

#include "masterizer/domain/dsp/Compressor.h"

namespace masterizer::domain::dsp {

/// Two-band compressor settings. Bands are split with a Linkwitz-Riley (LR4)
/// crossover at `crossoverHz`.
struct MultibandCompressorSettings {
    double crossoverHz = 200.0;
    CompressorSettings low;
    CompressorSettings high;
};

/// Splits into low/high bands, compresses each, and sums back in place.
void applyMultibandCompressor(float* samples, int count, double sampleRate,
                              const MultibandCompressorSettings& settings);

}  // namespace masterizer::domain::dsp
