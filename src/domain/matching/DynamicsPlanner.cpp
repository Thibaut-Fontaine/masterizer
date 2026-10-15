#include "masterizer/domain/matching/DynamicsPlanner.h"

#include "masterizer/domain/dsp/BpmRelease.h"

#include <vector>

namespace masterizer::domain::matching {
namespace {

constexpr double kGlueVariationGap = 0.05;
constexpr double kMultibandVariationGap = 0.2;
constexpr double kNeutralReleaseSeconds = 0.15;

double meanVariation(const std::vector<double>& variation) {
    if (variation.empty()) {
        return 0.0;
    }
    double sum = 0.0;
    for (double value : variation) {
        sum += value;
    }
    return sum / static_cast<double>(variation.size());
}

}  // namespace

DynamicsPlan planDynamics(const analysis::AnalysisProfile& source,
                          const analysis::AnalysisProfile& reference, double bpm) {
    DynamicsPlan plan;
    plan.suggestedReleaseSeconds =
        bpm > 0.0 ? dsp::suggestReleaseSeconds(bpm, 0.25) : kNeutralReleaseSeconds;

    const double gap = meanVariation(source.bandVariation) - meanVariation(reference.bandVariation);
    if (gap <= kGlueVariationGap) {
        return plan;
    }
    plan.glueEnabled = true;
    plan.glue.thresholdDb = -18.0;
    plan.glue.ratio = std::min(2.0, 1.5 + gap);
    plan.glue.attackSeconds = 0.02;
    plan.glue.releaseSeconds = plan.suggestedReleaseSeconds;
    plan.glue.kneeDb = 6.0;
    plan.multibandEnabled = gap > kMultibandVariationGap;
    return plan;
}

}  // namespace masterizer::domain::matching
