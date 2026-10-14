#pragma once

#include <vector>

namespace masterizer::test {

/// Repeated exponentially decaying spikes: high crest factor, drum-like.
std::vector<float> transientRich(int count, int interval, double decay, float amplitude);

/// A low sine plus its second harmonic: bass-heavy, low crest factor.
std::vector<float> bassHeavy(int count, double sampleRate, float amplitude);

}  // namespace masterizer::test
