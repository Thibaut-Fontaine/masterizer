#pragma once

#include <vector>

namespace masterizer::test {

/// `count` zero samples.
std::vector<float> silence(int count);

/// `count` zero samples with a single spike of `amplitude` at `position`.
std::vector<float> impulse(int count, int position, float amplitude);

/// `count` samples of a sine at `frequencyHz` over `sampleRate`.
std::vector<float> sine(int count, double frequencyHz, double sampleRate, float amplitude);

/// `count` deterministic uniform-noise samples in [-amplitude, amplitude].
std::vector<float> whiteNoise(int count, unsigned int seed, float amplitude);

}  // namespace masterizer::test
