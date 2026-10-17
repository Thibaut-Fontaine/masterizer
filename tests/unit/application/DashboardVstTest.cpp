#include "masterizer/application/DashboardModel.h"

#include "masterizer/application/MasterProfile.h"
#include "support/Checks.h"
#include "support/Profiles.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::application::buildDashboard;
using masterizer::application::DashboardInput;
using masterizer::application::DashboardModel;
using masterizer::application::MasterProfile;
using masterizer::domain::matching::CorrectionProposal;
using masterizer::domain::matching::CorrectionTarget;
using masterizer::test::profileWithBands;

TEST_CASE(band_confidence_comes_from_the_proposal) {
    const auto source = profileWithBands(std::vector<double>(10, 0.1));
    const auto reference = profileWithBands(std::vector<double>(10, 0.1));
    DashboardInput input;
    input.source = &source;
    input.reference = &reference;
    CorrectionProposal proposal;
    proposal.target = CorrectionTarget::Eq;
    proposal.band = 3;
    proposal.amountDb = -2.5;
    proposal.confidence = 0.7;
    input.proposals.push_back(proposal);
    const DashboardModel model = buildDashboard(input);
    CHECK_NEAR(model.bands[3].confidence, 0.7, 1e-12);
    CHECK_NEAR(model.bands[0].confidence, 0.0, 1e-12);
}

TEST_CASE(profile_and_module_activity_pass_through) {
    DashboardInput input;
    input.profile = MasterProfile::Loud;
    input.modules.push_back({"eq", true, -2.5});
    input.modules.push_back({"tape", false, 0.0});
    const DashboardModel model = buildDashboard(input);
    CHECK(model.profile == MasterProfile::Loud);
    CHECK(model.modules.size() == 2u);
    CHECK(model.modules[0].id == "eq");
    CHECK(model.modules[0].enabled);
    CHECK_NEAR(model.modules[0].amount, -2.5, 1e-12);
    CHECK(!model.modules[1].enabled);
}
