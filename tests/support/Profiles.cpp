#include "support/Profiles.h"
#include <vector>

namespace masterizer::test {

masterizer::domain::analysis::AnalysisProfile profileWithBands(
    const std::vector<double>& octaveEnergies) {
    masterizer::domain::analysis::AnalysisProfile profile;
    profile.octaveEnergy = octaveEnergies;
    return profile;
}

}  // namespace masterizer::test
