#pragma once

#include "masterizer/domain/analysis/AnalysisProfile.h"
#include "masterizer/domain/audio/AudioBuffer.h"
#include "masterizer/domain/dsp/MonoBass.h"
#include "masterizer/domain/dsp/Saturator.h"
#include "masterizer/domain/matching/CorrectionBounds.h"
#include "masterizer/domain/matching/CorrectionProposal.h"
#include "masterizer/domain/matching/ResonanceDetector.h"

#include <vector>

namespace masterizer::application {

/// Buffer-level tonal stages. All operate in place on every channel.
void applyMonoBassStage(domain::audio::AudioBuffer& buffer,
                        const domain::dsp::MonoBassSettings& settings);
void applySaturationStage(domain::audio::AudioBuffer& buffer,
                          const domain::dsp::SaturationSettings& settings);
void applyResonanceStage(domain::audio::AudioBuffer& buffer,
                         const domain::analysis::AnalysisProfile& source,
                         const domain::matching::ResonanceSettings& settings);
void applyEqStage(domain::audio::AudioBuffer& buffer,
                  const std::vector<domain::matching::CorrectionProposal>& proposals,
                  double intensity, const domain::matching::CorrectionBounds& bounds);
void applyStereoStage(domain::audio::AudioBuffer& buffer, double widthScale);

}  // namespace masterizer::application
