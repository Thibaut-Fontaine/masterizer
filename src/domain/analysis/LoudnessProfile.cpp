#include "masterizer/domain/analysis/LoudnessProfile.h"

#include "masterizer/domain/analysis/Loudness.h"
#include "masterizer/domain/audio/AudioBufferOps.h"
#include "masterizer/domain/dsp/Gain.h"
#include "masterizer/domain/dsp/Peak.h"
#include "masterizer/domain/dsp/TruePeak.h"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace masterizer::domain::analysis {
namespace {

constexpr double kSilenceDb = -144.0;

double peakAcross(const std::vector<std::vector<float>>& channels, bool interSample) {
    double peak = 0.0;
    for (const std::vector<float>& channel : channels) {
        const int frames = static_cast<int>(channel.size());
        const double value = interSample ? dsp::truePeak(channel.data(), frames)
                                         : dsp::peakAbs(channel.data(), frames);
        peak = std::max(peak, value);
    }
    return peak;
}

}  // namespace

LoudnessProfile analyzeLoudness(const audio::AudioBuffer& buffer) {
    LoudnessProfile profile;
    const int channels = audio::channelCount(buffer);
    const int frames = audio::frameCount(buffer);
    if (channels <= 0 || frames <= 0) {
        return profile;
    }
    if (channels >= 2) {
        profile.integratedLufs = integratedLufs(buffer.channels[0].data(),
                                                buffer.channels[1].data(), frames,
                                                buffer.sampleRate);
    } else {
        profile.integratedLufs =
            integratedLufs(buffer.channels[0].data(), frames, buffer.sampleRate);
    }
    const double truePeak = peakAcross(buffer.channels, true);
    const double samplePeak = peakAcross(buffer.channels, false);
    profile.truePeakDb = truePeak > 0.0 ? dsp::linearToDb(truePeak) : kSilenceDb;
    profile.samplePeakDb = samplePeak > 0.0 ? dsp::linearToDb(samplePeak) : kSilenceDb;
    return profile;
}

}  // namespace masterizer::domain::analysis
