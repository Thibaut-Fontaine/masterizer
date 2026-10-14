#include "masterizer/domain/analysis/SpectralFraming.h"

#include <cmath>
#include <cstddef>

namespace masterizer::domain::analysis {
namespace {

constexpr double kTwoPi = 2.0 * 3.14159265358979323846;

}  // namespace

bool isPowerOfTwo(int value) {
    return value > 0 && (value & (value - 1)) == 0;
}

std::vector<double> hannWindow(int size) {
    std::vector<double> window(static_cast<std::size_t>(size > 0 ? size : 0), 1.0);
    if (size < 2) {
        return window;
    }
    for (int i = 0; i < size; ++i) {
        window[static_cast<std::size_t>(i)] =
            0.5 - 0.5 * std::cos(kTwoPi * i / static_cast<double>(size - 1));
    }
    return window;
}

}  // namespace masterizer::domain::analysis
