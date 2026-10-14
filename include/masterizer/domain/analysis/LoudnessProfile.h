#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"

namespace masterizer::domain::analysis {

/// Full-signal loudness/peak summary. Loudness is proper BS.1770-4 gated LUFS;
/// peaks are reported in dBFS (true peak uses 4x oversampling).
struct LoudnessProfile {
    double integratedLufs = -70.0;
    double truePeakDb = -144.0;
    double samplePeakDb = -144.0;
};

/// Analyses a buffer (mono or stereo). Empty input yields default values.
LoudnessProfile analyzeLoudness(const audio::AudioBuffer& buffer);

}  // namespace masterizer::domain::analysis
