#pragma once

#include "masterizer/domain/analysis/AnalysisProfile.h"
#include "masterizer/domain/audio/AudioBuffer.h"

namespace masterizer::domain::analysis {

/// Analysis resolution controls. `fftSize` must be a power of two.
struct AnalysisSettings {
    int fftSize = 2048;
    int hop = 512;
};

/// Builds a complete AnalysisProfile from a buffer. Offline and allocation
/// heavy: never call from the audio thread.
AnalysisProfile analyzeSource(const audio::AudioBuffer& buffer, const AnalysisSettings& settings);

}  // namespace masterizer::domain::analysis
