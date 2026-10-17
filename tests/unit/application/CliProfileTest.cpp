#include "masterizer/application/CliOptions.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::application::CliOptions;
using masterizer::application::MasterProfile;
using masterizer::application::parseCliOptions;
using masterizer::application::ParseStatus;

TEST_CASE(the_profile_flag_selects_loud_or_soft) {
    CliOptions loud;
    CHECK(parseCliOptions({"cli", "r", "s", "o", "--profile", "loud"}, loud) ==
          ParseStatus::Ok);
    CHECK(loud.profile == MasterProfile::Loud);
    CHECK(!loud.both);

    CliOptions soft;
    CHECK(parseCliOptions({"cli", "r", "s", "o", "--profile", "soft"}, soft) ==
          ParseStatus::Ok);
    CHECK(soft.profile == MasterProfile::Soft);
}

TEST_CASE(both_renders_two_profiles) {
    CliOptions options;
    CHECK(parseCliOptions({"cli", "r", "s", "o", "--profile", "both"}, options) ==
          ParseStatus::Ok);
    CHECK(options.both);
}

TEST_CASE(a_ceiling_override_with_both_is_an_error) {
    CliOptions options;
    CHECK(parseCliOptions({"cli", "r", "s", "o", "--profile", "both", "--ceiling", "-0.5"},
                          options) == ParseStatus::Error);
    CHECK(parseCliOptions({"cli", "r", "s", "o", "--profile", "sideways"}, options) ==
          ParseStatus::Error);
}
