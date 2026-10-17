#include "masterizer/domain/dsp/TruePeak.h"

#include "masterizer/domain/dsp/Peak.h"

#include <array>
#include <cmath>
#include <cstddef>

namespace masterizer::domain::dsp {
namespace {

constexpr int kOversample = 4;
constexpr int kHalfTaps = 6;
constexpr int kTaps = 2 * kHalfTaps;
constexpr double kPi = 3.14159265358979323846;

/// Blackman-windowed sinc interpolation kernel with support |u| < kHalfTaps.
double kernel(double u) {
    const double support = static_cast<double>(kHalfTaps);
    if (std::fabs(u) >= support) {
        return 0.0;
    }
    const double sinc = std::fabs(u) < 1e-9 ? 1.0 : std::sin(kPi * u) / (kPi * u);
    const double window =
        0.42 + 0.5 * std::cos(kPi * u / support) + 0.08 * std::cos(2.0 * kPi * u / support);
    return sinc * window;
}

struct KernelTable {
    std::array<std::array<double, kTaps>, kOversample> weights{};
    std::array<double, kOversample> norms{};
};

KernelTable buildTable() {
    KernelTable table;
    for (int phase = 0; phase < kOversample; ++phase) {
        const double fraction = static_cast<double>(phase) / kOversample;
        double norm = 0.0;
        for (int tap = 0; tap < kTaps; ++tap) {
            const int k = tap - kHalfTaps + 1;
            const double weight = kernel(fraction - static_cast<double>(k));
            table.weights[static_cast<std::size_t>(phase)][static_cast<std::size_t>(tap)] = weight;
            norm += weight;
        }
        table.norms[static_cast<std::size_t>(phase)] = norm;
    }
    return table;
}

const KernelTable& table() {
    static const KernelTable value = buildTable();
    return value;
}

double sampleAt(const float* samples, int count, int index) {
    if (index < 0 || index >= count) {
        return 0.0;
    }
    return static_cast<double>(samples[index]);
}

}  // namespace

double truePeak(const float* samples, int count) {
    if (samples == nullptr || count <= 0) {
        return 0.0;
    }
    double peak = peakAbs(samples, count);
    const int margin = count > 2 * kHalfTaps ? kHalfTaps : 0;
    const KernelTable& kernelTable = table();
    for (int i = margin; i < count - margin; ++i) {
        for (int phase = 0; phase < kOversample; ++phase) {
            double accumulated = 0.0;
            int tap = 0;
            for (int k = -kHalfTaps + 1; k <= kHalfTaps; ++k, ++tap) {
                accumulated += kernelTable.weights[static_cast<std::size_t>(phase)]
                                                 [static_cast<std::size_t>(tap)] *
                               sampleAt(samples, count, i + k);
            }
            const double value =
                std::fabs(accumulated / kernelTable.norms[static_cast<std::size_t>(phase)]);
            if (value > peak) {
                peak = value;
            }
        }
    }
    return peak;
}

}  // namespace masterizer::domain::dsp
