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
#include <cstddef>
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

double globalCrestDb(const domain::audio::AudioBuffer& buffer) {
    double peak = 0.0;
    double sumSquares = 0.0;
    std::size_t count = 0;
    for (const std::vector<float>& channel : buffer.channels) {
        for (float sample : channel) {
            const double value = static_cast<double>(sample);
            peak = std::max(peak, std::fabs(value));
            sumSquares += value * value;
            ++count;
        }
    }
    if (sumSquares <= 0.0 || count == 0) {
        return 0.0;
    }
    const double rms = std::sqrt(sumSquares / static_cast<double>(count));
    return 20.0 * std::log10(peak / rms);
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
                                    bool limit, bool guardCrest, double referenceCrestDb,
                                    double maxCrestDropDb) {
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
    double bestLufs = achievedBefore;
    double bestError = std::numeric_limits<double>::infinity();
    bool foundValid = false;
    const double minimumCrest = referenceCrestDb - maxCrestDropDb;
    for (int iteration = 0; iteration < kIterations; ++iteration) {
        domain::audio::AudioBuffer candidate = original;
        applyGainAndLimit(candidate, gainDb, limiter, limit);
        enforceTruePeak(candidate, ceilingDb);
        const double lufs = measureLufs(candidate);
        const double crest = globalCrestDb(candidate);
        const bool valid = !guardCrest || crest >= minimumCrest;
        const double error =
            hasTarget && std::isfinite(lufs) ? std::fabs(lufs - targetLufs) : 0.0;
        if (valid && error < bestError) {
            bestError = error;
            bestGain = gainDb;
            bestLufs = lufs;
            foundValid = true;
        }
        if (!valid) {
            break;  // more gain only compresses further
        }
        if (!hasTarget || error <= kToleranceLu || !std::isfinite(lufs)) {
            break;
        }
        gainDb = std::clamp(gainDb + (targetLufs - lufs) * kRelaxation, 0.0, maxGain);
    }
    if (!foundValid) {
        bestGain = extraGainDb;  // the guard rejects any automatic gain
        bestLufs = achievedBefore;
    }

    buffer = original;
    applyGainAndLimit(buffer, bestGain, limiter, limit);
    enforceTruePeak(buffer, ceilingDb);

    result.appliedGainDb = bestGain;
    result.achievedLufs = bestLufs;
    result.truePeakDb = maxTruePeakDb(buffer);
    result.shortfallLu = std::isfinite(targetLufs) && std::isfinite(bestLufs)
                             ? std::max(0.0, targetLufs - bestLufs)
                             : 0.0;
    return result;
}

}  // namespace masterizer::application
