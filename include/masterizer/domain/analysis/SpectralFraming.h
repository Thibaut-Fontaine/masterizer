#pragma once

#include <vector>

namespace masterizer::domain::analysis {

/// True when `value` is a positive power of two.
bool isPowerOfTwo(int value);

/// Symmetric Hann window of the given size (size must be >= 2).
std::vector<double> hannWindow(int size);

}  // namespace masterizer::domain::analysis
