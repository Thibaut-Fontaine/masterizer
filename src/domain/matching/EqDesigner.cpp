#include "masterizer/domain/matching/EqDesigner.h"

#include "masterizer/domain/common/OctaveBands.h"

namespace masterizer::domain::matching {
namespace {

constexpr double kBandQ = 1.4;

}  // namespace

std::vector<dsp::EqBand> designEq(const std::vector<CorrectionProposal>& proposals) {
    std::vector<dsp::EqBand> bands;
    for (const CorrectionProposal& proposal : proposals) {
        if (proposal.target != CorrectionTarget::Eq || proposal.band < 0 ||
            proposal.band >= domain::bandCount()) {
            continue;
        }
        const auto octaveBand = static_cast<domain::OctaveBand>(proposal.band);
        const double centre = domain::centreHz(octaveBand);
        if (centre > 0.0) {
            bands.push_back({centre, kBandQ, proposal.amountDb});
        }
    }
    return bands;
}

}  // namespace masterizer::domain::matching
