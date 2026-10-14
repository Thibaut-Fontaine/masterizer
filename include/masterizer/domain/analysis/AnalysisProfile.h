#pragma once

#include "masterizer/domain/analysis/ProfileVersion.h"

namespace masterizer::domain::analysis {

/// Immutable, versioned analysis result. Fields are populated in later stages.
struct AnalysisProfile {
    ProfileVersion version = kCurrentProfileVersion;
    double integratedLufs = -70.0;
    double truePeakDb = -144.0;
};

}  // namespace masterizer::domain::analysis
