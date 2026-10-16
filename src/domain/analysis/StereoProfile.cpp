#include "masterizer/domain/analysis/StereoProfile.h"

#include "masterizer/domain/analysis/OctaveProfile.h"
#include "masterizer/domain/audio/AudioBufferOps.h"
#include "masterizer/domain/dsp/Correlation.h"

#include <cstddef>
#include <vector>

namespace masterizer::domain::analysis {
namespace {

std::vector<float> midChannel(const std::vector<float>& left, const std::vector<float>& right) {
    std::vector<float> mid(left.size());
    for (std::size_t i = 0; i < left.size(); ++i) {
        mid[i] = 0.5f * (left[i] + right[i]);
    }
    return mid;
}

std::vector<float> sideChannel(const std::vector<float>& left, const std::vector<float>& right) {
    std::vector<float> side(left.size());
    for (std::size_t i = 0; i < left.size(); ++i) {
        side[i] = 0.5f * (left[i] - right[i]);
    }
    return side;
}

double total(const std::vector<double>& values) {
    double sum = 0.0;
    for (double value : values) {
        sum += value;
    }
    return sum;
}

}  // namespace

StereoProfile analyzeStereo(const audio::AudioBuffer& buffer, int fftSize, int hop) {
    StereoProfile profile;
    if (audio::channelCount(buffer) < 2 || audio::frameCount(buffer) <= 0) {
        return profile;
    }
    const std::vector<float>& left = buffer.channels[0];
    const std::vector<float>& right = buffer.channels[1];
    const int frames = audio::frameCount(buffer);

    const std::vector<float> mid = midChannel(left, right);
    const std::vector<float> side = sideChannel(left, right);
    profile.midEnergy = octaveBandEnergies(mid.data(), frames, buffer.sampleRate, fftSize, hop);
    profile.sideEnergy = octaveBandEnergies(side.data(), frames, buffer.sampleRate, fftSize, hop);
    profile.correlation = dsp::correlation(left.data(), right.data(), frames);

    const double midTotal = total(profile.midEnergy);
    const double sideTotal = total(profile.sideEnergy);
    const double combined = midTotal + sideTotal;
    profile.width = combined > 0.0 ? sideTotal / combined : 0.0;
    return profile;
}

}  // namespace masterizer::domain::analysis
