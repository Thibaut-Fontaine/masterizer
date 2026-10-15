#include "masterizer/domain/matching/ChainCoordinator.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace masterizer::domain::matching {

std::vector<CorrectionProposal> coordinate(std::vector<CorrectionProposal> proposals,
                                           const CoordinationSettings& settings) {
    std::vector<CorrectionProposal> useful;
    for (const CorrectionProposal& proposal : proposals) {
        if (std::fabs(proposal.amountDb) >= settings.minUsefulDb) {
            useful.push_back(proposal);
        }
    }
    std::sort(useful.begin(), useful.end(), [](const CorrectionProposal& a,
                                               const CorrectionProposal& b) {
        return std::fabs(a.amountDb) > std::fabs(b.amountDb);
    });
    if (useful.size() > static_cast<std::size_t>(settings.maxProposals)) {
        useful.resize(static_cast<std::size_t>(settings.maxProposals));
    }
    double total = 0.0;
    for (const CorrectionProposal& proposal : useful) {
        total += std::fabs(proposal.amountDb);
    }
    if (total > settings.maxTotalEqDb) {
        const double factor = settings.maxTotalEqDb / total;
        for (CorrectionProposal& proposal : useful) {
            proposal.amountDb *= factor;
        }
    }
    return useful;
}

}  // namespace masterizer::domain::matching
