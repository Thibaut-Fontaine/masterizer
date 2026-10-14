#include "masterizer/domain/matching/ProfileDifference.h"

#include "support/Checks.h"
#include "support/Profiles.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::domain::matching::compareProfiles;
using masterizer::test::profileWithBands;

namespace {

const std::vector<double> kFlat(10, 0.1);

}  // namespace

TEST_CASE(identical_profiles_have_zero_difference) {
    const auto profile = profileWithBands(kFlat);
    const auto difference = compareProfiles(profile, profile);
    CHECK(difference.bands.size() == 10u);
    for (const auto& band : difference.bands) {
        CHECK_NEAR(band.differenceDb, 0.0, 1e-9);
    }
}

TEST_CASE(a_boosted_source_band_is_positive) {
    std::vector<double> source = kFlat;
    source[3] = 0.5;  // LowMid 180-360 Hz
    const auto difference = compareProfiles(profileWithBands(source), profileWithBands(kFlat));
    CHECK(difference.bands[3].differenceDb > 0.0);
    CHECK(difference.bands[0].differenceDb < 0.0);  // other bands lose relative share
}

TEST_CASE(tonal_comparison_is_independent_of_overall_level) {
    std::vector<double> loud = kFlat;
    for (double& energy : loud) {
        energy *= 4.0;  // same spectral shape, +6 dB overall
    }
    const auto difference = compareProfiles(profileWithBands(loud), profileWithBands(kFlat));
    for (const auto& band : difference.bands) {
        CHECK_NEAR(band.differenceDb, 0.0, 1e-9);
    }
}
