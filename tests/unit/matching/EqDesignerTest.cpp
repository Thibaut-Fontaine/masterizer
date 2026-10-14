#include "masterizer/domain/matching/EqDesigner.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::domain::matching::CorrectionProposal;
using masterizer::domain::matching::CorrectionTarget;
using masterizer::domain::matching::designEq;

namespace {

CorrectionProposal eqProposal(int band, double amountDb) {
    CorrectionProposal proposal;
    proposal.target = CorrectionTarget::Eq;
    proposal.band = band;
    proposal.amountDb = amountDb;
    return proposal;
}

}  // namespace

TEST_CASE(design_eq_maps_a_proposal_to_its_band_centre) {
    const auto bands = designEq({eqProposal(5, -3.0)});
    CHECK(bands.size() == 1u);
    CHECK_NEAR(bands[0].frequencyHz, 1018.2, 1.0);  // UpperMid 720-1440 Hz centre
    CHECK_NEAR(bands[0].gainDb, -3.0, 1e-9);
    CHECK(bands[0].q > 0.0);
}

TEST_CASE(design_eq_ignores_non_eq_and_invalid_proposals) {
    CorrectionProposal nonEq;
    nonEq.target = CorrectionTarget::Peak;
    nonEq.band = 2;
    const auto bands = designEq({nonEq, eqProposal(-1, 4.0)});
    CHECK(bands.empty());
}
