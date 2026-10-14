#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"

#include <vector>

namespace masterizer::domain::analysis {

/// Stereo statistics: per-band Mid and Side energy, correlation and width.
/// Width is side/(mid+side) over total band energy, bounded in [0, 1].
struct StereoProfile {
    std::vector<double> midEnergy;
    std::vector<double> sideEnergy;
    double correlation = 0.0;
    double width = 0.0;
};

/// Analyses a stereo buffer. Mono or empty input yields a default (zeroed) profile.
StereoProfile analyzeStereo(const audio::AudioBuffer& buffer, int fftSize, int hop);

}  // namespace masterizer::domain::analysis
