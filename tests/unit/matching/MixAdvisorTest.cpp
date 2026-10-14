#include "masterizer/domain/matching/MixAdvisor.h"

#include "support/Checks.h"
#include "support/Profiles.h"
#include "support/TestRegistry.h"

#include <string>
#include <vector>

using masterizer::domain::analysis::AnalysisProfile;
using masterizer::domain::matching::AdvisorRuleset;
using masterizer::domain::matching::advise;
using masterizer::domain::matching::MixSuggestion;
using masterizer::test::profileWithBands;

namespace {

const std::vector<double> kFlat(10, 0.1);

const auto hasId = [](const std::vector<MixSuggestion>& suggestions, const char* id) {
    for (const MixSuggestion& suggestion : suggestions) {
        if (suggestion.id == id) {
            return true;
        }
    }
    return false;
};

}  // namespace

TEST_CASE(identical_profiles_produce_no_suggestions) {
    const AnalysisProfile profile = profileWithBands(kFlat);
    CHECK(advise(profile, profile, AdvisorRuleset{}).empty());
}

TEST_CASE(over_compressed_source_reports_dynamics) {
    AnalysisProfile source = profileWithBands(kFlat);
    AnalysisProfile reference = profileWithBands(kFlat);
    source.envelope.crestDb = 3.0;
    reference.envelope.crestDb = 12.0;
    CHECK(hasId(advise(source, reference, AdvisorRuleset{}), "R3"));
}

TEST_CASE(missing_air_reports_tonal) {
    std::vector<double> bright = kFlat;
    bright[8] = 0.5;
    bright[9] = 0.5;
    const AnalysisProfile source = profileWithBands(kFlat);
    const AnalysisProfile reference = profileWithBands(bright);
    CHECK(hasId(advise(source, reference, AdvisorRuleset{}), "R5"));
}

TEST_CASE(narrow_image_reports_stereo) {
    AnalysisProfile source = profileWithBands(kFlat);
    AnalysisProfile reference = profileWithBands(kFlat);
    source.stereo.width = 0.1;
    reference.stereo.width = 0.5;
    CHECK(hasId(advise(source, reference, AdvisorRuleset{}), "R7"));
}

TEST_CASE(anti_phase_reports_phase_problem) {
    AnalysisProfile source = profileWithBands(kFlat);
    source.stereo.correlation = -0.5;
    CHECK(hasId(advise(source, profileWithBands(kFlat), AdvisorRuleset{}), "R11"));
}
