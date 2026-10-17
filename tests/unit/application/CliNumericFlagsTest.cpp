#include "masterizer/application/CliOptions.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::application::CliOptions;
using masterizer::application::parseCliOptions;
using masterizer::application::ParseStatus;

TEST_CASE(intensity_and_ceiling_do_not_leak_into_output_gain) {
    CliOptions options;
    CHECK(parseCliOptions({"cli", "r", "s", "o", "--intensity", "0.5", "--ceiling", "-2"},
                          options) == ParseStatus::Ok);
    CHECK_NEAR(options.matchIntensity, 0.5, 1e-12);
    CHECK_NEAR(options.ceilingDb, -2.0, 1e-12);
    CHECK_NEAR(options.outputGainDb, 0.0, 1e-12);
}

TEST_CASE(gain_still_parses_as_output_gain) {
    CliOptions options;
    CHECK(parseCliOptions({"cli", "r", "s", "o", "--gain", "3"}, options) == ParseStatus::Ok);
    CHECK_NEAR(options.outputGainDb, 3.0, 1e-12);
}

TEST_CASE(combined_numeric_flags_keep_their_own_values) {
    CliOptions options;
    CHECK(parseCliOptions({"cli", "r", "s", "o", "--intensity", "0.5", "--gain", "3",
                           "--ceiling", "-1.5"},
                          options) == ParseStatus::Ok);
    CHECK_NEAR(options.matchIntensity, 0.5, 1e-12);
    CHECK_NEAR(options.outputGainDb, 3.0, 1e-12);
    CHECK_NEAR(options.ceilingDb, -1.5, 1e-12);
}
