#include "masterizer/application/ChainReporter.h"

#include "support/Checks.h"
#include "support/Profiles.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::application::compareToReference;
using masterizer::application::ImprovementReport;
using masterizer::test::profileWithBands;

namespace {

std::vector<double> tilted(double lowMid, double upperMid) {
    std::vector<double> energies(10, 0.1);
    energies[3] = lowMid;
    energies[5] = upperMid;
    return energies;
}

}  // namespace

TEST_CASE(processing_equal_to_the_reference_is_a_perfect_match) {
    const auto reference = profileWithBands(tilted(0.3, 0.5));
    const ImprovementReport report =
        compareToReference(profileWithBands(tilted(0.1, 0.1)), reference, reference);
    CHECK_NEAR(report.distanceAfter, 0.0, 1e-9);
    CHECK(report.improved);
}

TEST_CASE(unchanged_processing_is_not_an_improvement) {
    const auto source = profileWithBands(tilted(0.1, 0.1));
    const ImprovementReport report =
        compareToReference(source, source, profileWithBands(tilted(0.3, 0.5)));
    CHECK_NEAR(report.distanceBefore, report.distanceAfter, 1e-9);
    CHECK(!report.improved);
}

TEST_CASE(a_partial_move_toward_the_reference_counts_as_improved) {
    const ImprovementReport report = compareToReference(
        profileWithBands(tilted(0.1, 0.1)), profileWithBands(tilted(0.25, 0.4)),
        profileWithBands(tilted(0.3, 0.5)));
    CHECK(report.distanceAfter < report.distanceBefore);
    CHECK(report.improved);
}
