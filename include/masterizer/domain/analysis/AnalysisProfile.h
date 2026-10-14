#pragma once

#include "masterizer/domain/analysis/EnvelopeProfile.h"
#include "masterizer/domain/analysis/LoudnessProfile.h"
#include "masterizer/domain/analysis/ProfileVersion.h"
#include "masterizer/domain/analysis/StereoProfile.h"

#include <vector>

namespace masterizer::domain::analysis {

/// Immutable, versioned set of analysis results for one signal.
struct AnalysisProfile {
    ProfileVersion version = kCurrentProfileVersion;
    std::vector<double> octaveEnergy;  // per octave band, index = int(OctaveBand)
    std::vector<double> spectrum;      // fine-resolution magnitude spectrum
    EnvelopeProfile envelope;
    StereoProfile stereo;
    LoudnessProfile loudness;
};

}  // namespace masterizer::domain::analysis
