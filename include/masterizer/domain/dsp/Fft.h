#pragma once

#include <complex>
#include <vector>

namespace masterizer::domain::dsp {

/// In-place iterative radix-2 (Cooley-Tukey) FFT. `data.size()` must be a power
/// of two; otherwise the input is left unchanged.
void fft(std::vector<std::complex<double>>& data);

}  // namespace masterizer::domain::dsp
