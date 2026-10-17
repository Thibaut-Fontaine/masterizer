#include "masterizer/application/LoudnessTargeter.h"

#include "masterizer/application/TruePeakGuard.h"
#include "masterizer/domain/analysis/Loudness.h"
#include "masterizer/domain/audio/AudioBufferOps.h"
#include "masterizer/domain/dsp/Gain.h"
#include "masterizer/domain/dsp/GainStage.h"
#include "masterizer/domain/dsp/Limiter.h"
#include "masterizer/domain/dsp/TruePeak.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace masterizer::application {
namespace {

/// Integrated LUFS only (no True Peak pass). This is the hot measurement in the
/// targeting loop, so it deliberately avoids the expensive peak scan.
double measureLufs(const domain::audio::AudioBuffer& buffer) {
    const int frames = domain::audio::frameCount(buffer);
    if (frames <= 0) {
        return -std::numeric_limits<double>::infinity();
    }
    if (buffer.channels.size() >= 2) {
        return domain::analysis::integratedLufs(buffer.channels[0].data(),
                                                buffer.channels[1].data(), frames,
                                                buffer.sampleRate);
    }
    return domain::analysis::integratedLufs(buffer.channels[0].data(), frames,
                                            buffer.sampleRate);
}

double maxTruePeakDb(const domain::audio::AudioBuffer& buffer) {
    double peak = 0.0;
    for (const std::vector<float>& channel : buffer.channels) {
        peak = std::max(peak, domain::dsp::truePeak(channel.data(),
                                                    static_cast<int>(channel.size())));
    }
    return peak > 0.0 ? domain::dsp::linearToDb(peak) : -144.0;
}

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
    const double achievedBefore = measureLufs(original);

    const bool hasTarget = std::isfinite(targetLufs) && std::isfinite(achievedBefore);
    const double maxGain = maxGainDb + std::max(0.0, extraGainDb);
    double gainDb = extraGainDb;
    if (hasTarget) {
        gainDb += std::clamp(targetLufs - achievedBefore, 0.0, maxGainDb);
    }

    domain::dsp::LimiterSettings limiter;
    limiter.ceilingDb = ceilingDb;

    double bestGain = gainDb;
    double bestError = std::numeric_limits<double>::infinity();
    for (int iteration = 0; iteration < kIterations; ++iteration) {
        domain::audio::AudioBuffer candidate = original;
        applyGainAndLimit(candidate, gainDb, limiter, limit);
        enforceTruePeak(candidate, ceilingDb);
        const double lufs = measureLufs(candidate);
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

    const double achieved = measureLufs(buffer);
    result.appliedGainDb = bestGain;
    result.achievedLufs = achieved;
    result.truePeakDb = maxTruePeakDb(buffer);
    result.shortfallLu = std::isfinite(targetLufs) && std::isfinite(achieved)
                             ? std::max(0.0, targetLufs - achieved)
                             : 0.0;
    return result;
}

}  // namespace masterizer::application
