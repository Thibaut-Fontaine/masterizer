#include "masterizer/application/LoudnessTargeter.h"

#include "masterizer/application/TruePeakGuard.h"
#include "masterizer/domain/analysis/LoudnessProfile.h"
#include "masterizer/domain/dsp/GainStage.h"
#include "masterizer/domain/dsp/Limiter.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace masterizer::application {
namespace {

void applyGainAndLimit(domain::audio::AudioBuffer& buffer, double gainDb,
                       const domain::dsp::LimiterSettings& limiter, bool limit) {
    for (std::vector<float>& channel : buffer.channels) {
        const int frames = static_cast<int>(channel.size());
        if (gainDb != 0.0) {
            domain::dsp::applyGain(channel.data(), frames, gainDb);
        }
        if (limit) {
            domain::dsp::applyLimiter(channel.data(), frames, buffer.sampleRate, limiter);
        }
    }
}

}  // namespace

LoudnessTargetResult targetLoudness(domain::audio::AudioBuffer& buffer, double targetLufs,
                                    double ceilingDb, double maxGainDb, double extraGainDb,
                                    bool limit) {
    LoudnessTargetResult result;
    const double achievedBefore =
        domain::analysis::analyzeLoudness(buffer).integratedLufs;

    double gainDb = extraGainDb;
    if (std::isfinite(targetLufs) && std::isfinite(achievedBefore)) {
        gainDb += std::clamp(targetLufs - achievedBefore, 0.0, maxGainDb);
    }

    domain::dsp::LimiterSettings limiter;
    limiter.ceilingDb = ceilingDb;
    applyGainAndLimit(buffer, gainDb, limiter, limit);
    enforceTruePeak(buffer, ceilingDb);

    const domain::analysis::LoudnessProfile after = domain::analysis::analyzeLoudness(buffer);
    result.appliedGainDb = gainDb;
    result.achievedLufs = after.integratedLufs;
    result.truePeakDb = after.truePeakDb;
    result.shortfallLu =
        std::isfinite(after.integratedLufs)
            ? std::max(0.0, targetLufs - after.integratedLufs)
            : 0.0;
    return result;
}

}  // namespace masterizer::application
