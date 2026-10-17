#include "masterizer/application/LoudnessTargeter.h"

#include "masterizer/application/TruePeakGuard.h"
#include "masterizer/domain/analysis/LoudnessProfile.h"
#include "masterizer/domain/dsp/GainStage.h"
#include "masterizer/domain/dsp/Limiter.h"

#include <algorithm>
#include <cmath>
#include <limits>
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

constexpr int kIterations = 5;
constexpr double kToleranceLu = 0.05;
constexpr double kRelaxation = 0.9;

}  // namespace

LoudnessTargetResult targetLoudness(domain::audio::AudioBuffer& buffer, double targetLufs,
                                    double ceilingDb, double maxGainDb, double extraGainDb,
                                    bool limit) {
    LoudnessTargetResult result;
    const domain::audio::AudioBuffer original = buffer;
    const double achievedBefore = domain::analysis::analyzeLoudness(original).integratedLufs;

    const bool hasTarget = std::isfinite(targetLufs) && std::isfinite(achievedBefore);
    const double maxGain = maxGainDb + std::max(0.0, extraGainDb);
    double gainDb = extraGainDb;
    if (hasTarget) {
        gainDb += std::clamp(targetLufs - achievedBefore, 0.0, maxGainDb);
    }

    domain::dsp::LimiterSettings limiter;
    limiter.ceilingDb = ceilingDb;

    // Closed loop: the limiter and True Peak guard eat some of the gain, so a
    // single pre-limiter calculation undershoots. Search a few gain candidates.
    double bestGain = gainDb;
    double bestError = std::numeric_limits<double>::infinity();
    for (int iteration = 0; iteration < kIterations; ++iteration) {
        domain::audio::AudioBuffer candidate = original;
        applyGainAndLimit(candidate, gainDb, limiter, limit);
        enforceTruePeak(candidate, ceilingDb);
        const double lufs = domain::analysis::analyzeLoudness(candidate).integratedLufs;
        const double error =
            hasTarget && std::isfinite(lufs) ? std::fabs(lufs - targetLufs) : 0.0;
        if (error < bestError) {
            bestError = error;
            bestGain = gainDb;
        }
        if (!hasTarget || error <= kToleranceLu || !std::isfinite(lufs)) {
            break;
        }
        gainDb = std::clamp(gainDb + (targetLufs - lufs) * kRelaxation, 0.0, maxGain);
    }

    buffer = original;
    applyGainAndLimit(buffer, bestGain, limiter, limit);
    enforceTruePeak(buffer, ceilingDb);

    const domain::analysis::LoudnessProfile after = domain::analysis::analyzeLoudness(buffer);
    result.appliedGainDb = bestGain;
    result.achievedLufs = after.integratedLufs;
    result.truePeakDb = after.truePeakDb;
    result.shortfallLu = std::isfinite(targetLufs) && std::isfinite(after.integratedLufs)
                             ? std::max(0.0, targetLufs - after.integratedLufs)
                             : 0.0;
    return result;
}

}  // namespace masterizer::application
