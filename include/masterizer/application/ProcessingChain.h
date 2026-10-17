#pragma once

#include "masterizer/domain/analysis/AnalysisProfile.h"
#include "masterizer/domain/audio/AudioBuffer.h"
#include "masterizer/domain/dsp/Compressor.h"
#include "masterizer/domain/dsp/DynamicEq.h"
#include "masterizer/domain/dsp/Limiter.h"
#include "masterizer/domain/dsp/MonoBass.h"
#include "masterizer/domain/dsp/MultibandCompressor.h"
#include "masterizer/domain/dsp/Saturator.h"
#include "masterizer/domain/dsp/TapeStage.h"
#include "masterizer/domain/dsp/TransientShaper.h"
#include "masterizer/domain/matching/CorrectionBounds.h"
#include "masterizer/domain/matching/CorrectionProposal.h"
#include "masterizer/domain/matching/ResonanceDetector.h"

#include <vector>

namespace masterizer::application {

/// Full-chain settings. Every module has its own enable flag.
struct ChainSettings {
    domain::matching::CorrectionBounds bounds;
    domain::matching::ResonanceSettings resonance;
    domain::dsp::MonoBassSettings monoBass;
    domain::dsp::SaturationSettings saturation;
    domain::dsp::TapeSettings tape;
    domain::dsp::TransientShaperSettings transient;
    domain::dsp::CompressorSettings glue;
    domain::dsp::MultibandCompressorSettings multiband;
    domain::dsp::DynamicEqBandSettings dynamicEq;
    domain::dsp::LimiterSettings limiter;
    double matchIntensity = 1.0;
    double outputGainDb = 0.0;
    double stereoWidthScale = 1.0;
    bool eqEnabled = true;
    bool fineEqEnabled = false;
    bool monoBassEnabled = true;
    bool saturationEnabled = false;
    bool tapeEnabled = false;
    bool resonanceEnabled = false;
    bool stereoEnabled = false;
    bool transientEnabled = false;
    bool glueEnabled = false;
    bool multibandEnabled = false;
    bool dynamicEqEnabled = false;
    bool limiterEnabled = true;
};

/// The processed buffer plus the corrections that were proposed.
struct ChainResult {
    domain::audio::AudioBuffer buffer;
    std::vector<domain::matching::CorrectionProposal> proposals;
};

/// Applies the coordinated chain to `input`, driven by the Source/Reference
/// profiles. Order: mono/M-S, saturation, de-resonator, tonal EQ, stereo,
/// transient, glue, multiband, dynamic EQ, output. Offline; not RT-safe.
ChainResult process(const domain::audio::AudioBuffer& input,
                    const domain::analysis::AnalysisProfile& source,
                    const domain::analysis::AnalysisProfile& reference,
                    const ChainSettings& settings);

}  // namespace masterizer::application
