#include "support/Signals.h"

#include <cmath>
#include <random>

namespace masterizer::test {
namespace {

constexpr double kTwoPi = 2.0 * 3.14159265358979323846;

std::size_t toSize(int count) {
    return count > 0 ? static_cast<std::size_t>(count) : 0u;
}

}  // namespace

std::vector<float> silence(int count) {
    return std::vector<float>(toSize(count), 0.0f);
}

std::vector<float> impulse(int count, int position, float amplitude) {
    std::vector<float> data(toSize(count), 0.0f);
    if (position >= 0 && position < count) {
        data[static_cast<std::size_t>(position)] = amplitude;
    }
    return data;
}

std::vector<float> sine(int count, double frequencyHz, double sampleRate, float amplitude) {
    std::vector<float> data(toSize(count), 0.0f);
    const double step = kTwoPi * frequencyHz / sampleRate;
    for (int i = 0; i < count; ++i) {
        data[static_cast<std::size_t>(i)] = static_cast<float>(amplitude * std::sin(step * i));
    }
    return data;
}

std::vector<float> whiteNoise(int count, unsigned int seed, float amplitude) {
    std::vector<float> data(toSize(count), 0.0f);
    std::mt19937 generator(seed);
    std::uniform_real_distribution<float> distribution(-amplitude, amplitude);
    for (int i = 0; i < count; ++i) {
        data[static_cast<std::size_t>(i)] = distribution(generator);
    }
    return data;
}

}  // namespace masterizer::test
