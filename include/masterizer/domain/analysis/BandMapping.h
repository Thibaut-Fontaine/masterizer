#pragma once

namespace masterizer::domain::analysis {

/// Octave band index for a frequency in Hz, or -1 when it falls outside all bands.
int bandIndexForFrequency(double frequency);

}  // namespace masterizer::domain::analysis
