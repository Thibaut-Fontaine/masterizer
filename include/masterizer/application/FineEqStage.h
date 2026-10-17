#pragma once

#include "masterizer/domain/analysis/AnalysisProfile.h"
#include "masterizer/domain/audio/AudioBuffer.h"
#include "masterizer/domain/matching/CorrectionBounds.h"

namespace masterizer::application {

/// Applies the third-octave fine EQ derived from Source/Reference spectra.
void applyFineEqStage(domain::audio::AudioBuffer& buffer,
                      const domain::analysis::AnalysisProfile& source,
                      const domain::analysis::AnalysisProfile& reference,
                      const domain::matching::CorrectionBounds& bounds);

}  // namespace masterizer::application
