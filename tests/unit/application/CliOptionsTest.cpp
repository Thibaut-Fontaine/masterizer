#include "masterizer/application/CliOptions.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <string>
#include <vector>

using masterizer::application::CliOptions;
using masterizer::application::parseCliOptions;
using masterizer::application::ParseStatus;

TEST_CASE(parses_three_paths_and_options) {
    CliOptions options;
    const auto status = parseCliOptions({"cli", "ref.wav", "src.wav", "out.wav",
                                         "--intensity", "0.5", "--ceiling", "-2"},
                                        options);
    CHECK(status == ParseStatus::Ok);
    CHECK(options.referencePath == "ref.wav");
    CHECK(options.outputPath == "out.wav");
    CHECK_NEAR(options.matchIntensity, 0.5, 1e-12);
    CHECK_NEAR(options.ceilingDb, -2.0, 1e-12);
}

TEST_CASE(module_flags_set_overrides) {
    CliOptions options;
    parseCliOptions({"cli", "r.wav", "s.wav", "o.wav", "--no-eq", "--glue"}, options);
    CHECK(options.moduleOverrides["eq"] == false);
    CHECK(options.moduleOverrides["glue"] == true);
}

TEST_CASE(ab_and_delta_flags_are_recognised) {
    CliOptions options;
    parseCliOptions({"cli", "r.wav", "s.wav", "o.wav", "--ab", "--delta", "--timing",
                     "--no-crest-guard"},
                    options);
    CHECK(options.writeAb);
    CHECK(options.writeDelta);
    CHECK(options.timing);
    CHECK(!options.crestGuard);
}

TEST_CASE(help_is_reported) {
    CliOptions options;
    CHECK(parseCliOptions({"cli", "--help"}, options) == ParseStatus::Help);
}

TEST_CASE(missing_paths_or_unknown_flags_are_errors) {
    CliOptions options;
    CHECK(parseCliOptions({"cli", "r.wav"}, options) == ParseStatus::Error);
    CHECK(parseCliOptions({"cli", "r.wav", "s.wav", "o.wav", "--nonsense"}, options) ==
          ParseStatus::Error);
}
