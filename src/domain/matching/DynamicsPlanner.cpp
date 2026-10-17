#include "masterizer/domain/matching/DynamicsPlanner.h"

#include "masterizer/domain/dsp/BpmRelease.h"

#include <cstddef>
#include <vector>
#include <algorithm>

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

double bandAverage(const std::vector<double>& variation, int firstBand, int lastBand) {
    if (variation.empty()) {
        return 0.0;
    }
    double sum = 0.0;
    int count = 0;
    for (int band = firstBand; band <= lastBand; ++band) {
        if (band >= 0 && static_cast<std::size_t>(band) < variation.size()) {
            sum += variation[static_cast<std::size_t>(band)];
            ++count;
        }
    }
    return count > 0 ? sum / count : 0.0;
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

    // The diagnostic showed the human master controls 1.4-5.8 kHz dynamics more
    // than we do. Propose a gentle dynamic EQ there when our upper-mid variation
    // exceeds the reference.
    const double highMidGap =
        bandAverage(source.bandVariation, 5, 7) - bandAverage(reference.bandVariation, 5, 7);
    if (highMidGap > kGlueVariationGap) {
        plan.dynamicEqEnabled = true;
        plan.dynamicEq.frequencyHz = 3000.0;
        plan.dynamicEq.q = 1.0;
        plan.dynamicEq.thresholdDb = -18.0;
        plan.dynamicEq.maxCutDb = 2.0;
        plan.dynamicEq.attackSeconds = 0.01;
        plan.dynamicEq.releaseSeconds = 0.1;
    }
    return plan;
}

}  // namespace masterizer::domain::matching
