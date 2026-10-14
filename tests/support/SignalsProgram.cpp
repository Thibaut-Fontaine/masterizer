#include "support/SignalsProgram.h"

#include <cmath>

namespace masterizer::test {
namespace {

constexpr double kTwoPi = 2.0 * 3.14159265358979323846;

std::size_t toSize(int count) {
    return count > 0 ? static_cast<std::size_t>(count) : 0u;
}

}  // namespace

std::vector<float> transientRich(int count, int interval, double decay, float amplitude) {
    std::vector<float> data(toSize(count), 0.0f);
    if (interval <= 0) {
        return data;
    }
    double level = 0.0;
    for (int i = 0; i < count; ++i) {
        level = (i % interval == 0) ? static_cast<double>(amplitude) : level * decay;
        data[static_cast<std::size_t>(i)] = static_cast<float>(level);
    }
    return data;
}

std::vector<float> bassHeavy(int count, double sampleRate, float amplitude) {
    std::vector<float> data(toSize(count), 0.0f);
    for (int i = 0; i < count; ++i) {
        const double phase = kTwoPi * 45.0 * static_cast<double>(i) / sampleRate;
        const double value = std::sin(phase) + 0.3 * std::sin(2.0 * phase);
        data[static_cast<std::size_t>(i)] = static_cast<float>(amplitude * value / 1.3);
    }
    return data;
}

}  // namespace masterizer::test
