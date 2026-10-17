#pragma once

#include "masterizer/application/ProcessingChain.h"
#include "masterizer/domain/dsp/CompressorProcessor.h"
#include "masterizer/domain/dsp/DynamicEqProcessor.h"
#include "masterizer/domain/dsp/EqProcessor.h"
#include "masterizer/domain/dsp/LookaheadProcessor.h"
#include "masterizer/domain/dsp/MonoBassProcessor.h"
#include "masterizer/domain/dsp/MultibandProcessor.h"
#include "masterizer/domain/dsp/TapeProcessor.h"
#include "masterizer/domain/dsp/TransientProcessor.h"

#include <optional>
#include <vector>

namespace masterizer::application {

/// Resolved EQ bands for the real-time chain. The plugin computes these from
/// the proposals/fine spectra; the chain itself does not run analysis.
struct RealtimeChainBands {
    std::vector<domain::dsp::EqBand> resonance;
    std::vector<domain::dsp::EqBand> eq;
    std::vector<domain::dsp::EqBand> fineEq;
};

/// Block-based counterpart of `process()`. Every module owns its state, so the
/// chain can be called with any host block size. Construction allocates; `process`
/// does not. The output gain is applied before the look-ahead limiter, matching
/// the loudness targeter rather than the legacy `applyOutputStage` order.
class RealtimeChain {
public:
    RealtimeChain(double sampleRate, int maxBlockSize, const ChainSettings& settings,
                  const RealtimeChainBands& bands);
    int latencySamples() const;
    void process(float* left, float* right, int count);

private:
    std::optional<domain::dsp::MonoBassProcessor> monoBass_;
    domain::dsp::SaturationSettings saturation_;
    bool saturationEnabled_ = false;
    std::optional<domain::dsp::EqProcessor> resonanceLeft_;
    std::optional<domain::dsp::EqProcessor> resonanceRight_;
    std::optional<domain::dsp::EqProcessor> eqLeft_;
    std::optional<domain::dsp::EqProcessor> eqRight_;
    std::optional<domain::dsp::EqProcessor> fineEqLeft_;
    std::optional<domain::dsp::EqProcessor> fineEqRight_;
    bool stereoEnabled_ = false;
    double stereoWidthScale_ = 1.0;
    std::optional<domain::dsp::TransientProcessor> transientLeft_;
    std::optional<domain::dsp::TransientProcessor> transientRight_;
    std::optional<domain::dsp::CompressorProcessor> glueLeft_;
    std::optional<domain::dsp::CompressorProcessor> glueRight_;
    std::optional<domain::dsp::MultibandProcessor> multibandLeft_;
    std::optional<domain::dsp::MultibandProcessor> multibandRight_;
    std::optional<domain::dsp::DynamicEqProcessor> dynamicEqLeft_;
    std::optional<domain::dsp::DynamicEqProcessor> dynamicEqRight_;
    std::optional<domain::dsp::TapeProcessor> tapeLeft_;
    std::optional<domain::dsp::TapeProcessor> tapeRight_;
    double outputGainDb_ = 0.0;
    std::optional<domain::dsp::LookaheadProcessor> limiterLeft_;
    std::optional<domain::dsp::LookaheadProcessor> limiterRight_;
};

}  // namespace masterizer::application
