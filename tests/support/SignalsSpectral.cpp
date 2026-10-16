#include "support/SignalsSpectral.h"

#include "support/Signals.h"

#include <cmath>
#include <cstddef>
#include <vector>

namespace masterizer::test {
namespace {

constexpr double kTwoPi = 2.0 * 3.14159265358979323846;

std::size_t toSize(int count) {
    return count > 0 ? static_cast<std::size_t>(count) : 0u;
}

}  // namespace

std::vector<float> sweepLog(int count, double startHz, double endHz,
                            double sampleRate, float amplitude) {
    std::vector<float> data(toSize(count), 0.0f);
    if (count <= 0) {
        return data;
    }
    const double duration = static_cast<double>(count) / sampleRate;
    const double ratio = std::log(endHz / startHz);
    for (int i = 0; i < count; ++i) {
        const double t = static_cast<double>(i) / sampleRate;
        const double phase = kTwoPi * startHz * duration / ratio *
                             (std::exp(ratio * t / duration) - 1.0);
        data[static_cast<std::size_t>(i)] = static_cast<float>(amplitude * std::sin(phase));
    }
    return data;
}

std::vector<float> pinkNoise(int count, unsigned int seed, float amplitude) {
    std::vector<float> data(toSize(count), 0.0f);
    const auto white = whiteNoise(count, seed, 1.0f);
    double b0 = 0.0;
    double b1 = 0.0;
    double b2 = 0.0;
    double b3 = 0.0;
    double b4 = 0.0;
    double b5 = 0.0;
    double b6 = 0.0;
    for (int i = 0; i < count; ++i) {
        const double w = static_cast<double>(white[static_cast<std::size_t>(i)]);
        b0 = 0.99886 * b0 + w * 0.0555179;
        b1 = 0.99332 * b1 + w * 0.0750759;
        b2 = 0.96900 * b2 + w * 0.1538520;
        b3 = 0.86650 * b3 + w * 0.3104856;
        b4 = 0.55000 * b4 + w * 0.5329522;
        b5 = -0.7616 * b5 - w * 0.0168980;
        const double pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + w * 0.5362;
        b6 = w * 0.115926;
        data[static_cast<std::size_t>(i)] = static_cast<float>(amplitude * pink * 0.11);
    }
    return data;
}

}  // namespace masterizer::test
