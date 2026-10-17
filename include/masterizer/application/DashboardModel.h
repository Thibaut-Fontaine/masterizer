#pragma once

#include "masterizer/application/OutputValidator.h"
#include "masterizer/domain/analysis/AnalysisProfile.h"
#include "masterizer/domain/matching/CorrectionProposal.h"
#include "masterizer/domain/matching/MixSuggestion.h"
#include "masterizer/domain/state/ReferenceStatus.h"

#include <string>
#include <vector>

namespace masterizer::application {

/// Display-ready values for one octave band.
struct BandView {
    int band = 0;
    double sourceDb = -120.0;
    double referenceDb = -120.0;
    double differenceDb = 0.0;   // source - reference (relative, level-normalised)
    double correctionDb = 0.0;   // applied EQ amount, if any
};

/// Everything the editor needs to render one frame. Pure data; no drawing.
struct DashboardModel {
    domain::state::ReferenceStatus state = domain::state::ReferenceStatus::Unloaded;
    std::string referenceName;
    std::vector<BandView> bands;
    double sourceLufs = -70.0;
    double referenceLufs = -70.0;
    double truePeakDb = -144.0;
    double matchIntensity = 1.0;
    bool abMonitoring = false;
    bool deltaMonitoring = false;
    std::vector<domain::matching::MixSuggestion> suggestions;
    std::vector<std::string> warnings;
};

/// Inputs to the presenter. Profiles may be null (nothing analysed yet).
struct DashboardInput {
    const domain::analysis::AnalysisProfile* source = nullptr;
    const domain::analysis::AnalysisProfile* reference = nullptr;
    std::vector<domain::matching::CorrectionProposal> proposals;
    domain::state::ReferenceStatus state = domain::state::ReferenceStatus::Unloaded;
    std::string referenceName;
    OutputReport report;
    double matchIntensity = 1.0;
    bool abMonitoring = false;
    bool deltaMonitoring = false;
    std::vector<domain::matching::MixSuggestion> suggestions;
};

/// Builds the view model. Pure and deterministic.
DashboardModel buildDashboard(const DashboardInput& input);

}  // namespace masterizer::application
