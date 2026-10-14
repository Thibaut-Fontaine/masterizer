#pragma once

namespace masterizer::domain::matching {

/// Configurable thresholds for the Mix Advisor rules. A data table, not
/// hard-coded branches, so tuning does not require logic changes.
struct AdvisorRuleset {
    int version = 1;
    double tonalDb = 2.5;            // band relative-level imbalance (dB)
    double variationGap = 0.15;      // band variation shortfall
    double densityGap = 0.5;         // band transient-density gap (onsets/s)
    double crestGapDb = 4.0;         // global crest shortfall (dB)
    double widthGap = 0.15;          // stereo width gap
    double correlationLow = 0.3;     // low correlation (mono/phase risk)
    double correlationNegative = -0.2;  // negative correlation (phase problem)
    double peakiness = 8.0;          // narrow-peak indicator
    double persistence = 0.4;        // resonance persistence fraction
};

}  // namespace masterizer::domain::matching
