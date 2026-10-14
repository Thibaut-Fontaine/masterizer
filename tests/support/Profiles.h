#pragma once

#include "masterizer/domain/analysis/AnalysisProfile.h"

#include <vector>

namespace masterizer::test {

/// Builds a profile whose octave energies are exactly the given values.
masterizer::domain::analysis::AnalysisProfile profileWithBands(
    const std::vector<double>& octaveEnergies);

}  // namespace masterizer::test
