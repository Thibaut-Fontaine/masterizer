#include "masterizer/domain/matching/ProfileDifference.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace masterizer::domain::matching {
namespace {

constexpr double kFloorDb = -120.0;

/// Relative band levels (dB) after normalising the profile to its own total
/// energy, so the comparison is independent of overall loudness.
std::vector<double> relativeBandLevelsDb(const analysis::AnalysisProfile& profile) {
    std::vector<double> levels(profile.octaveEnergy.size(), kFloorDb);
    double total = 0.0;
    for (double energy : profile.octaveEnergy) {
        total += energy;
    }
    if (total <= 0.0) {
        return levels;
    }
    for (std::size_t band = 0; band < profile.octaveEnergy.size(); ++band) {
        const double ratio = profile.octaveEnergy[band] / total;
        levels[band] = ratio > 0.0 ? 10.0 * std::log10(ratio) : kFloorDb;
    }
    return levels;
}

double finiteDifference(double a, double b) {
    const double difference = a - b;
    return std::isfinite(difference) ? difference : 0.0;
}

}  // namespace

ProfileDifference compareProfiles(const analysis::AnalysisProfile& source,
                                  const analysis::AnalysisProfile& reference) {
    ProfileDifference difference;
    const std::vector<double> sourceLevels = relativeBandLevelsDb(source);
    const std::vector<double> referenceLevels = relativeBandLevelsDb(reference);
    const std::size_t bands = std::min(sourceLevels.size(), referenceLevels.size());
    difference.bands.reserve(bands);
    for (std::size_t band = 0; band < bands; ++band) {
        difference.bands.push_back(
            {static_cast<int>(band), sourceLevels[band] - referenceLevels[band]});
    }
    difference.loudnessDifferenceLu =
        finiteDifference(source.loudness.integratedLufs, reference.loudness.integratedLufs);
    difference.transientDifference = source.envelope.percussiveProminence -
                                     reference.envelope.percussiveProminence;
    difference.widthDifference = source.stereo.width - reference.stereo.width;
    difference.truePeakDifferenceDb =
        finiteDifference(source.loudness.truePeakDb, reference.loudness.truePeakDb);
    return difference;
}

}  // namespace masterizer::domain::matching
