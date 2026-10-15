#include "masterizer/domain/matching/ChainCoordinator.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <cmath>
#include <vector>

using masterizer::domain::matching::CoordinationSettings;
using masterizer::domain::matching::coordinate;
using masterizer::domain::matching::CorrectionProposal;
using masterizer::domain::matching::CorrectionTarget;

namespace {

CorrectionProposal eq(int band, double amountDb) {
    CorrectionProposal proposal;
    proposal.target = CorrectionTarget::Eq;
    proposal.band = band;
    proposal.amountDb = amountDb;
    return proposal;
}

}  // namespace

TEST_CASE(insignificant_proposals_are_dropped) {
    const auto result = coordinate({eq(1, 0.2), eq(2, 4.0)}, CoordinationSettings{});
    CHECK(result.size() == 1u);
    CHECK(result[0].band == 2);
}

TEST_CASE(the_number_of_proposals_is_capped) {
    std::vector<CorrectionProposal> proposals;
    for (int band = 0; band < 10; ++band) {
        proposals.push_back(eq(band, 3.0));
    }
    const auto result = coordinate(proposals, CoordinationSettings{});
    CHECK(result.size() == 6u);
}

TEST_CASE(total_correction_magnitude_is_bounded) {
    std::vector<CorrectionProposal> proposals;
    for (int band = 0; band < 5; ++band) {
        proposals.push_back(eq(band, 6.0));  // 30 dB total
    }
    const auto result = coordinate(proposals, CoordinationSettings{});
    double total = 0.0;
    for (const CorrectionProposal& proposal : result) {
        total += std::fabs(proposal.amountDb);
    }
    CHECK(total <= 12.0 + 1e-9);
}

TEST_CASE(strongest_proposals_come_first) {
    const auto result = coordinate({eq(1, 2.0), eq(2, 6.0), eq(3, 4.0)}, CoordinationSettings{});
    CHECK(result[0].band == 2);
    CHECK(result[1].band == 3);
    CHECK(result[2].band == 1);
}
