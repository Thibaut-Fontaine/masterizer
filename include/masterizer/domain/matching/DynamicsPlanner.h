#pragma once

#include "masterizer/domain/analysis/AnalysisProfile.h"
#include "masterizer/domain/dsp/Compressor.h"

namespace masterizer::domain::matching {

/// Dynamics treatment proposed from a Source/Reference comparison.
struct DynamicsPlan {
    bool glueEnabled = false;
    domain::dsp::CompressorSettings glue;
    bool multibandEnabled = false;
    double suggestedReleaseSeconds = 0.15;
};

/// Plans gentle dynamics control from the difference in per-band variation.
/// `bpm` <= 0 disables tempo-synced release (a neutral default is used).
DynamicsPlan planDynamics(const analysis::AnalysisProfile& source,
                          const analysis::AnalysisProfile& reference, double bpm);

}  // namespace masterizer::domain::matching
