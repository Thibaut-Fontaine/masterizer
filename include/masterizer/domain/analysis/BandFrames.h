#pragma once

#include <vector>

namespace masterizer::domain::analysis {

/// Per-frame, per-band power from the STFT: rows are frames, columns are the
/// octave bands. Empty for invalid parameters or a signal shorter than one frame.
std::vector<std::vector<double>> frameBandEnergies(const float* samples, int count,
                                                   double sampleRate, int fftSize, int hop);

}  // namespace masterizer::domain::analysis
