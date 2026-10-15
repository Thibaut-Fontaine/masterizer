#include "masterizer/application/DashboardModel.h"

#include "masterizer/domain/common/OctaveBands.h"
#include "support/Checks.h"
#include "support/Profiles.h"
#include "support/TestRegistry.h"

#include <string>
#include <vector>

using masterizer::application::buildDashboard;
using masterizer::application::DashboardInput;
using masterizer::application::DashboardModel;
using masterizer::domain::bandCount;
using masterizer::domain::matching::CorrectionProposal;
using masterizer::domain::matching::CorrectionTarget;
using masterizer::domain::state::ReferenceStatus;
using masterizer::test::profileWithBands;

TEST_CASE(a_ready_dashboard_exposes_every_band) {
    const auto source = profileWithBands(std::vector<double>(10, 0.1));
    const auto reference = profileWithBands(std::vector<double>(10, 0.1));
    DashboardInput input;
    input.source = &source;
    input.reference = &reference;
    input.state = ReferenceStatus::Ready;
    const DashboardModel model = buildDashboard(input);
    CHECK(model.bands.size() == static_cast<std::size_t>(bandCount()));
    CHECK(model.warnings.empty());
    CHECK(model.state == ReferenceStatus::Ready);
}

TEST_CASE(a_correction_is_shown_on_its_band) {
    const auto source = profileWithBands(std::vector<double>(10, 0.1));
    const auto reference = profileWithBands(std::vector<double>(10, 0.1));
    DashboardInput input;
    input.source = &source;
    input.reference = &reference;
    input.state = ReferenceStatus::Ready;
    CorrectionProposal proposal;
    proposal.target = CorrectionTarget::Eq;
    proposal.band = 3;
    proposal.amountDb = -2.5;
    input.proposals.push_back(proposal);
    const DashboardModel model = buildDashboard(input);
    CHECK_NEAR(model.bands[3].correctionDb, -2.5, 1e-12);
    CHECK_NEAR(model.bands[0].correctionDb, 0.0, 1e-12);
}

TEST_CASE(an_unloaded_reference_warns) {
    DashboardInput input;
    const DashboardModel model = buildDashboard(input);
    CHECK(!model.warnings.empty());
    CHECK(model.bands.empty());
}

TEST_CASE(a_stale_reference_warns_to_re_analyse) {
    DashboardInput input;
    input.state = ReferenceStatus::Stale;
    const DashboardModel model = buildDashboard(input);
    bool found = false;
    for (const std::string& warning : model.warnings) {
        if (warning.find("re-analyse") != std::string::npos) {
            found = true;
        }
    }
    CHECK(found);
}

TEST_CASE(an_unreachable_target_and_a_ceiling_breach_both_warn) {
    const auto source = profileWithBands(std::vector<double>(10, 0.1));
    const auto reference = profileWithBands(std::vector<double>(10, 0.1));
    DashboardInput input;
    input.source = &source;
    input.reference = &reference;
    input.state = ReferenceStatus::Ready;
    input.report.shortfallLu = 1.5;
    input.report.ceilingExceeded = true;
    const DashboardModel model = buildDashboard(input);
    CHECK(model.warnings.size() >= 2u);
}
