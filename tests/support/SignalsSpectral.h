#pragma once

#include <vector>

namespace masterizer::test {

/// Logarithmic sweep from `startHz` to `endHz` over `sampleRate`.
std::vector<float> sweepLog(int count, double startHz, double endHz,
                            double sampleRate, float amplitude);

/// Deterministic pink-ish noise (Paul Kellet filter) in the spirit of -3 dB/oct.
std::vector<float> pinkNoise(int count, unsigned int seed, float amplitude);

}  // namespace masterizer::test
