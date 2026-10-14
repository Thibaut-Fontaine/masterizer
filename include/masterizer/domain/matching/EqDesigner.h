#pragma once

#include "masterizer/domain/dsp/ParametricEq.h"
#include "masterizer/domain/matching/CorrectionProposal.h"

#include <vector>

namespace masterizer::domain::matching {

/// Converts tonal (EQ) correction proposals into peaking bands placed at the
/// octave-band geometric centre frequencies. Non-EQ or out-of-range proposals
/// are ignored.
std::vector<dsp::EqBand> designEq(const std::vector<CorrectionProposal>& proposals);

}  // namespace masterizer::domain::matching
