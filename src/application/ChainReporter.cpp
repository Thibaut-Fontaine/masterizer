#include "masterizer/application/ChainReporter.h"

#include "masterizer/domain/matching/ProfileDifference.h"

#include <cmath>

namespace masterizer::application {
namespace {

double spectralDistance(const domain::analysis::AnalysisProfile& a,
                        const domain::analysis::AnalysisProfile& b) {
    const domain::matching::ProfileDifference difference = domain::matching::compareProfiles(a, b);
    double sum = 0.0;
    for (const auto& band : difference.bands) {
        sum += std::fabs(band.differenceDb);
    }
    return sum;
}

}  // namespace

ImprovementReport compareToReference(const domain::analysis::AnalysisProfile& source,
                                     const domain::analysis::AnalysisProfile& processed,
                                     const domain::analysis::AnalysisProfile& reference) {
    ImprovementReport report;
    report.distanceBefore = spectralDistance(source, reference);
    report.distanceAfter = spectralDistance(processed, reference);
    report.improved = report.distanceAfter < report.distanceBefore;
    return report;
}

}  // namespace masterizer::application
