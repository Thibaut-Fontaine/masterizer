#pragma once

#include <vector>

namespace masterizer::test {

/// A left/right sample pair of equal length.
struct StereoPair {
    std::vector<float> left;
    std::vector<float> right;
};

/// Two channels of independent noise: uncorrelated, wide-stereo material.
StereoPair wideStereo(int count, unsigned int seed, float amplitude);

/// Identical channels with opposite polarity: correlation -1, mono-destructive.
StereoPair antiPhase(int count, double frequencyHz, double sampleRate, float amplitude);

}  // namespace masterizer::test
