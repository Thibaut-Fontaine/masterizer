#pragma once

#include "masterizer/domain/analysis/AnalysisProfile.h"

#include <vector>

namespace masterizer::domain::matching {

/// Tonal (relative, level-normalised) difference for one octave band.
/// Positive means the Source is louder than the Reference in that band.
struct BandDifference {
    int band = 0;
    double differenceDb = 0.0;
};

/// Difference between a Source and a Reference profile.
struct ProfileDifference {
    std::vector<BandDifference> bands;  // per octave band
    double loudnessDifferenceLu = 0.0;
    double transientDifference = 0.0;
    double widthDifference = 0.0;
    double truePeakDifferenceDb = 0.0;
};

/// Compares two profiles. Tonal bands are compared as relative band levels
/// (each profile normalised to its own total energy), so a globally louder
/// Reference does not bias the tonal comparison.
ProfileDifference compareProfiles(const analysis::AnalysisProfile& source,
                                  const analysis::AnalysisProfile& reference);

}  // namespace masterizer::domain::matching
