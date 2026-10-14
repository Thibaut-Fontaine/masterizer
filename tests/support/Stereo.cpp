#include "support/Stereo.h"

#include "support/Signals.h"

namespace masterizer::test {

StereoPair wideStereo(int count, unsigned int seed, float amplitude) {
    StereoPair pair;
    pair.left = whiteNoise(count, seed, amplitude);
    pair.right = whiteNoise(count, seed + 1u, amplitude);
    return pair;
}

StereoPair antiPhase(int count, double frequencyHz, double sampleRate, float amplitude) {
    StereoPair pair;
    pair.left = sine(count, frequencyHz, sampleRate, amplitude);
    pair.right = pair.left;
    for (float& sample : pair.right) {
        sample = -sample;
    }
    return pair;
}

}  // namespace masterizer::test
