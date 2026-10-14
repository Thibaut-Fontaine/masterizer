#pragma once

namespace masterizer::domain::dsp {

/// Crest factor in dB, 20*log10(peak/rms). Returns 0 for silence or empty buffers.
double crestFactorDb(const float* samples, int count);

}  // namespace masterizer::domain::dsp
