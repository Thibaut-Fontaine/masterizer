#pragma once

#include "masterizer/domain/analysis/AnalysisProfile.h"
#include "masterizer/domain/audio/AudioBuffer.h"
#include "masterizer/domain/dsp/Limiter.h"
#include "masterizer/domain/dsp/MonoBass.h"
#include "masterizer/domain/matching/CorrectionBounds.h"
#include "masterizer/domain/matching/CorrectionProposal.h"

#include <vector>

namespace masterizer::application {

/// Per-module switches and settings for the processing chain.
struct ChainSettings {
    domain::matching::CorrectionBounds bounds;
    domain::dsp::MonoBassSettings monoBass;
    domain::dsp::LimiterSettings limiter;
    double matchIntensity = 1.0;
    bool eqEnabled = true;
    bool monoBassEnabled = true;
    bool limiterEnabled = true;
};

/// The processed buffer plus the corrections that were proposed.
struct ChainResult {
    domain::audio::AudioBuffer buffer;
    std::vector<domain::matching::CorrectionProposal> proposals;
};

/// Applies the coordinated chain (EQ -> low-end M/S -> limiter) to `input`,
/// driven by the Source/Reference profiles. Offline; not audio-thread safe.
ChainResult process(const domain::audio::AudioBuffer& input,
                    const domain::analysis::AnalysisProfile& source,
                    const domain::analysis::AnalysisProfile& reference,
                    const ChainSettings& settings);

}  // namespace masterizer::application
