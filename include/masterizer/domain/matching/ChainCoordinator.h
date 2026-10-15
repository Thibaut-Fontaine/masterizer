#pragma once

#include "masterizer/domain/matching/CorrectionProposal.h"

#include <vector>

namespace masterizer::domain::matching {

/// Processing budget that penalises unnecessary or excessive correction.
struct CoordinationSettings {
    int maxProposals = 6;
    double maxTotalEqDb = 12.0;
    double minUsefulDb = 0.5;
};

/// Drops sub-threshold corrections, keeps the strongest ones, and bounds the
/// total EQ magnitude. Deterministic; the result is ordered strongest first.
std::vector<CorrectionProposal> coordinate(std::vector<CorrectionProposal> proposals,
                                           const CoordinationSettings& settings);

}  // namespace masterizer::domain::matching
