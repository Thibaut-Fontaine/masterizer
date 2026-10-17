#include "masterizer/application/MasterProfile.h"
#include "masterizer/application/Render.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <string>

using masterizer::application::ceilingDbFor;
using masterizer::application::MasterProfile;
using masterizer::application::masterOutputPath;
using masterizer::application::profileName;

TEST_CASE(profile_ceilings_are_soft_minus_one_and_loud_minus_point_one) {
    CHECK_NEAR(ceilingDbFor(MasterProfile::Soft), -1.0, 1e-12);
    CHECK_NEAR(ceilingDbFor(MasterProfile::Loud), -0.1, 1e-12);
}

TEST_CASE(profile_names_and_output_paths) {
    CHECK(std::string(profileName(MasterProfile::Soft)) == "soft");
    CHECK(std::string(profileName(MasterProfile::Loud)) == "loud");
    CHECK(masterOutputPath("out.wav", MasterProfile::Soft) == "out.soft.wav");
    CHECK(masterOutputPath("out.wav", MasterProfile::Loud) == "out.loud.wav");
    CHECK(masterOutputPath("out", MasterProfile::Loud) == "out.loud");
}
