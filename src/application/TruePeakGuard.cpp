#include "masterizer/application/TruePeakGuard.h"

#include "masterizer/domain/dsp/Gain.h"
#include "masterizer/domain/dsp/GainStage.h"
#include "masterizer/domain/dsp/TruePeak.h"

#include <algorithm>
#include <vector>

namespace masterizer::application {
namespace {

double maxTruePeakDb(const domain::audio::AudioBuffer& buffer) {
    double peak = 0.0;
    for (const std::vector<float>& channel : buffer.channels) {
        peak = std::max(peak,
                        domain::dsp::truePeak(channel.data(), static_cast<int>(channel.size())));
    }
    return peak > 0.0 ? domain::dsp::linearToDb(peak) : -144.0;
}

}  // namespace

double enforceTruePeak(domain::audio::AudioBuffer& buffer, double ceilingDb) {
    const double measured = maxTruePeakDb(buffer);
    if (measured <= ceilingDb) {
        return 0.0;
    }
    const double gainDb = ceilingDb - measured;
    for (std::vector<float>& channel : buffer.channels) {
        domain::dsp::applyGain(channel.data(), static_cast<int>(channel.size()), gainDb);
    }
    return gainDb;
}

}  // namespace masterizer::application
