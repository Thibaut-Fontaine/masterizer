#include "masterizer/domain/matching/EqDesigner.h"

#include "masterizer/domain/common/OctaveBands.h"

#include <vector>

namespace masterizer::domain::matching {
namespace {

constexpr double kBandQ = 1.4;
constexpr double kSubShelfHz = 40.0;
constexpr double kAirShelfHz = 8000.0;

}  // namespace

std::vector<dsp::EqBand> designEq(const std::vector<CorrectionProposal>& proposals) {
    std::vector<dsp::EqBand> bands;
    for (const CorrectionProposal& proposal : proposals) {
        if (proposal.target != CorrectionTarget::Eq || proposal.band < 0 ||
            proposal.band >= domain::bandCount()) {
            continue;
        }
        if (proposal.band == 0) {
            bands.push_back({kSubShelfHz, kBandQ, proposal.amountDb, dsp::EqShape::LowShelf});
            continue;
        }
        if (proposal.band == domain::bandCount() - 1) {
            bands.push_back({kAirShelfHz, kBandQ, proposal.amountDb, dsp::EqShape::HighShelf});
            continue;
        }
        const auto octaveBand = static_cast<domain::OctaveBand>(proposal.band);
        const double centre = domain::centreHz(octaveBand);
        if (centre > 0.0) {
            bands.push_back({centre, kBandQ, proposal.amountDb, dsp::EqShape::Peaking});
        }
    }
    return bands;
}

}  // namespace masterizer::domain::matching
