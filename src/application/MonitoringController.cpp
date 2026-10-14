#include "masterizer/application/MonitoringController.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace masterizer::application {

double loudnessMatchGainDb(double originalLufs, double processedLufs) {
    if (!std::isfinite(originalLufs) || !std::isfinite(processedLufs)) {
        return 0.0;
    }
    return originalLufs - processedLufs;
}

domain::audio::AudioBuffer differenceSignal(const domain::audio::AudioBuffer& processed,
                                            const domain::audio::AudioBuffer& original) {
    domain::audio::AudioBuffer result = processed;
    const std::size_t channels =
        std::min(processed.channels.size(), original.channels.size());
    for (std::size_t channel = 0; channel < channels; ++channel) {
        const std::size_t frames = std::min(processed.channels[channel].size(),
                                            original.channels[channel].size());
        for (std::size_t i = 0; i < frames; ++i) {
            result.channels[channel][i] =
                processed.channels[channel][i] - original.channels[channel][i];
        }
    }
    return result;
}

}  // namespace masterizer::application
