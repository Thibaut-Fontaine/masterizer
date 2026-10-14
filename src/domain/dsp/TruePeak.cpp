#include "masterizer/domain/dsp/TruePeak.h"

#include "masterizer/domain/dsp/Peak.h"

#include <cmath>

namespace masterizer::domain::dsp {
namespace {

constexpr int kOversample = 4;
constexpr int kHalfTaps = 6;
constexpr double kPi = 3.14159265358979323846;

/// Blackman-windowed sinc interpolation kernel with support |u| < kHalfTaps.
double kernel(double u) {
    const double support = static_cast<double>(kHalfTaps);
    if (std::fabs(u) >= support) {
        return 0.0;
    }
    const double sinc = std::fabs(u) < 1e-9 ? 1.0 : std::sin(kPi * u) / (kPi * u);
    const double window = 0.42 + 0.5 * std::cos(kPi * u / support) +
                          0.08 * std::cos(2.0 * kPi * u / support);
    return sinc * window;
}

double sampleAt(const float* samples, int count, int index) {
    if (index < 0 || index >= count) {
        return 0.0;
    }
    return static_cast<double>(samples[index]);
}

/// Value at `base + phase / kOversample`, normalised so constants are preserved.
double interpolate(const float* samples, int count, int base, int phase) {
    const double fraction = static_cast<double>(phase) / kOversample;
    double weighted = 0.0;
    double normalization = 0.0;
    for (int k = -kHalfTaps + 1; k <= kHalfTaps; ++k) {
        const double weight = kernel(fraction - static_cast<double>(k));
        weighted += weight * sampleAt(samples, count, base + k);
        normalization += weight;
    }
    return normalization != 0.0 ? weighted / normalization : 0.0;
}

}  // namespace

double truePeak(const float* samples, int count) {
    if (samples == nullptr || count <= 0) {
        return 0.0;
    }
    // Always include the sampled peak so the result can never under-read it.
    double peak = peakAbs(samples, count);
    // Skip the edge margin where the interpolation kernel is not fully supported,
    // and where zero-padding would create spurious ringing.
    const int margin = count > 2 * kHalfTaps ? kHalfTaps : 0;
    for (int i = margin; i < count - margin; ++i) {
        for (int phase = 0; phase < kOversample; ++phase) {
            const double value = std::fabs(interpolate(samples, count, i, phase));
            if (value > peak) {
                peak = value;
            }
        }
    }
    return peak;
}

}  // namespace masterizer::domain::dsp
