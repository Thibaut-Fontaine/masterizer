#include "masterizer/domain/matching/MatchingEngine.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace masterizer::domain::matching {
namespace {

constexpr double kFullConfidenceDb = 6.0;

double tonalConfidence(double differenceDb) {
    return std::min(1.0, std::fabs(differenceDb) / kFullConfidenceDb);
}

/// The correction applied to the Source band: negative = cut, positive = boost.
double boundedAmount(double correctionDb, const CorrectionBounds& bounds) {
    if (correctionDb < -bounds.maxEqCutDb) {
        return -bounds.maxEqCutDb;
    }
    if (correctionDb > bounds.maxEqBoostDb) {
        return bounds.maxEqBoostDb;
    }
    return correctionDb;
}

}  // namespace

MatchingEngine::MatchingEngine(CorrectionBounds bounds) : bounds_(bounds) {}

std::vector<CorrectionProposal> MatchingEngine::propose(
    const ProfileDifference& difference) const {
    std::vector<CorrectionProposal> proposals;
    for (const BandDifference& band : difference.bands) {
        if (std::fabs(band.differenceDb) < bounds_.minEqDifferenceDb) {
            continue;
        }
        const double confidence = tonalConfidence(band.differenceDb);
        if (confidence < bounds_.minConfidence) {
            continue;
        }
        CorrectionProposal proposal;
        proposal.target = CorrectionTarget::Eq;
        proposal.band = band.band;
        proposal.amountDb = boundedAmount(-band.differenceDb, bounds_);
        proposal.direction = proposal.amountDb < 0.0 ? CorrectionDirection::Cut
                                                     : CorrectionDirection::Boost;
        proposal.confidence = confidence;
        proposal.reason = "tonal_balance";
        proposals.push_back(proposal);
    }
    return proposals;
}

}  // namespace masterizer::domain::matching
