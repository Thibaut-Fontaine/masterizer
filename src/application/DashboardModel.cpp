#include "masterizer/application/DashboardModel.h"

#include "masterizer/domain/common/OctaveBands.h"
#include "masterizer/domain/matching/ProfileDifference.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

namespace masterizer::application {
namespace {

constexpr double kFloor = 1e-12;

double toDb(double energy) {
    return energy > kFloor ? 10.0 * std::log10(energy) : -120.0;
}

std::vector<BandView> buildBands(const DashboardInput& input) {
    std::vector<BandView> bands;
    if (input.source == nullptr || input.reference == nullptr) {
        return bands;
    }
    const auto difference =
        domain::matching::compareProfiles(*input.source, *input.reference);
    const int count = domain::bandCount();
    for (int band = 0; band < count; ++band) {
        BandView view;
        view.band = band;
        if (static_cast<std::size_t>(band) < input.source->octaveEnergy.size()) {
            view.sourceDb = toDb(input.source->octaveEnergy[static_cast<std::size_t>(band)]);
        }
        if (static_cast<std::size_t>(band) < input.reference->octaveEnergy.size()) {
            view.referenceDb = toDb(input.reference->octaveEnergy[static_cast<std::size_t>(band)]);
        }
        if (static_cast<std::size_t>(band) < difference.bands.size()) {
            view.differenceDb = difference.bands[static_cast<std::size_t>(band)].differenceDb;
        }
        for (const domain::matching::CorrectionProposal& proposal : input.proposals) {
            if (proposal.target == domain::matching::CorrectionTarget::Eq &&
                proposal.band == band) {
                view.correctionDb = proposal.amountDb;
                view.confidence = proposal.confidence;
            }
        }
        bands.push_back(view);
    }
    return bands;
}

std::vector<std::string> buildWarnings(const DashboardInput& input) {
    std::vector<std::string> warnings;
    switch (input.state) {
        case domain::state::ReferenceStatus::Unloaded:
            warnings.emplace_back("No reference loaded.");
            break;
        case domain::state::ReferenceStatus::Loading:
            warnings.emplace_back("Analysing reference...");
            break;
        case domain::state::ReferenceStatus::Stale:
            warnings.emplace_back("Reference changed; re-analyse before trusting the match.");
            break;
        case domain::state::ReferenceStatus::Failed:
            warnings.emplace_back("Reference failed to load.");
            break;
        case domain::state::ReferenceStatus::Ready:
            break;
    }
    if (input.report.shortfallLu > 0.5) {
        warnings.push_back("Target loudness not reached (short by " +
                           std::to_string(input.report.shortfallLu).substr(0, 4) + " LU).");
    }
    if (input.report.ceilingExceeded) {
        warnings.emplace_back("True Peak ceiling exceeded.");
    }
    return warnings;
}

}  // namespace

DashboardModel buildDashboard(const DashboardInput& input) {
    DashboardModel model;
    model.state = input.state;
    model.referenceName = input.referenceName;
    model.bands = buildBands(input);
    if (input.source != nullptr) {
        model.sourceLufs = input.source->loudness.integratedLufs;
    }
    if (input.reference != nullptr) {
        model.referenceLufs = input.reference->loudness.integratedLufs;
    }
    model.truePeakDb = input.report.truePeakDb;
    model.matchIntensity = input.matchIntensity;
    model.abMonitoring = input.abMonitoring;
    model.deltaMonitoring = input.deltaMonitoring;
    model.profile = input.profile;
    model.modules = input.modules;
    model.suggestions = input.suggestions;
    model.warnings = buildWarnings(input);
    return model;
}

}  // namespace masterizer::application
