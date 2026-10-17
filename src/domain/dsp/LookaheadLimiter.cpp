#include "masterizer/domain/dsp/LookaheadLimiter.h"

#include "masterizer/domain/dsp/Gain.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <deque>
#include <vector>

namespace masterizer::domain::dsp {

void applyLookaheadLimiter(float* samples, int count, double sampleRate,
                           const LookaheadLimiterSettings& settings) {
    if (samples == nullptr || count <= 0 || sampleRate <= 0.0) {
        return;
    }
    const double ceiling = dbToLinear(settings.ceilingDb);
    const int lookahead =
        std::max(1, static_cast<int>(std::lround(settings.lookaheadSeconds * sampleRate)));

    // Target gain per sample from the maximum magnitude in the look-ahead window.
    std::vector<double> target(static_cast<std::size_t>(count), 1.0);
    std::deque<int> window;
    int next = 0;
    for (int i = 0; i < count; ++i) {
        const int end = std::min(count, i + lookahead);
        while (next < end) {
            const double value = std::fabs(static_cast<double>(samples[next]));
            while (!window.empty() &&
                   std::fabs(static_cast<double>(samples[window.back()])) <= value) {
                window.pop_back();
            }
            window.push_back(next);
            ++next;
        }
        while (!window.empty() && window.front() < i) {
            window.pop_front();
        }
        const double peak =
            window.empty() ? 0.0 : std::fabs(static_cast<double>(samples[window.front()]));
        target[static_cast<std::size_t>(i)] = peak > 0.0 ? std::min(1.0, ceiling / peak) : 1.0;
    }

    const double releaseStep =
        1.0 - std::exp(-1.0 / (std::max(settings.releaseSeconds, 1e-6) * sampleRate));
    double gain = 1.0;
    for (int i = 0; i < count; ++i) {
        const double wanted = target[static_cast<std::size_t>(i)];
        if (wanted < gain) {
            gain = wanted;  // anticipatory: no overshoot
        } else {
            gain += releaseStep * (1.0 - gain);
        }
        samples[i] = static_cast<float>(static_cast<double>(samples[i]) * gain);
    }
}

}  // namespace masterizer::domain::dsp
