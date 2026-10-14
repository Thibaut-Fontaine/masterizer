#pragma once

#include "masterizer/domain/matching/CorrectionBounds.h"
#include "masterizer/domain/matching/CorrectionProposal.h"
#include "masterizer/domain/matching/ProfileDifference.h"

#include <vector>

namespace masterizer::domain::matching {

/// Turns a profile difference into bounded, confidence-tagged corrections.
/// Stage 5 MVP proposes tonal (EQ) corrections; it never modifies audio.
class MatchingEngine {
public:
    explicit MatchingEngine(CorrectionBounds bounds);

    /// Proposed corrections for review, in band order. Empty when nothing is
    /// confident or significant enough to act on.
    std::vector<CorrectionProposal> propose(const ProfileDifference& difference) const;

private:
    CorrectionBounds bounds_;
};

}  // namespace masterizer::domain::matching
