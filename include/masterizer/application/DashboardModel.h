#pragma once

#include "masterizer/application/MasterProfile.h"
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
    double confidence = 0.0;     // proposal confidence, 0 when there is no correction
};

/// One processing module as the editor should display it.
struct ModuleView {
    std::string id;
    bool enabled = false;
    double amount = 0.0;  // correction amount / activity, module-defined
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
    MasterProfile profile = MasterProfile::Soft;
    std::vector<ModuleView> modules;
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
    MasterProfile profile = MasterProfile::Soft;
    std::vector<ModuleView> modules;
    std::vector<domain::matching::MixSuggestion> suggestions;
};

/// Builds the view model. Pure and deterministic.
DashboardModel buildDashboard(const DashboardInput& input);

}  // namespace masterizer::application
