#pragma once

#include "masterizer/domain/matching/CorrectionBounds.h"

namespace masterizer::domain::matching {

/// Scales a proposed EQ amount by Match Intensity (0..2). 0% is neutral, 100%
/// reproduces the proposal, above 100% strengthens it -- always within bounds.
/// This is a module-specific mapping, not a blanket parameter multiply.
double scaleEqAmount(double amountDb, double intensity, const CorrectionBounds& bounds);

}  // namespace masterizer::domain::matching
