#pragma once

#include "masterizer/domain/analysis/AnalysisProfile.h"

namespace masterizer::application {

/// Whether processing moved the Source closer to the Reference.
struct ImprovementReport {
    double distanceBefore = 0.0;
    double distanceAfter = 0.0;
    bool improved = false;
};

/// Compares processed-vs-reference spectral distance against
/// source-vs-reference, using the same level-normalised per-band basis as the
/// Matching Engine. A lower distance does not by itself prove better sound.
ImprovementReport compareToReference(const domain::analysis::AnalysisProfile& source,
                                     const domain::analysis::AnalysisProfile& processed,
                                     const domain::analysis::AnalysisProfile& reference);

}  // namespace masterizer::application
