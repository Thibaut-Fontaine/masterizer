#include "masterizer/domain/dsp/Fft.h"

#include "masterizer/domain/common/Types.h"

#include <cmath>
#include <cstddef>
#include <utility>

namespace masterizer::domain::dsp {
namespace {

bool isPowerOfTwo(std::size_t value) {
    return value != 0 && (value & (value - 1)) == 0;
}

void bitReverse(std::vector<std::complex<double>>& data) {
    const std::size_t n = data.size();
    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; (j & bit) != 0; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
}

void butterflies(std::vector<std::complex<double>>& data) {
    const std::size_t n = data.size();
    for (std::size_t length = 2; length <= n; length <<= 1) {
        const double angle = -2.0 * domain::kPi / static_cast<double>(length);
        const std::complex<double> root(std::cos(angle), std::sin(angle));
        for (std::size_t start = 0; start < n; start += length) {
            std::complex<double> factor(1.0, 0.0);
            for (std::size_t k = 0; k < length / 2; ++k) {
                const std::complex<double> even = data[start + k];
                const std::complex<double> odd = data[start + k + length / 2] * factor;
                data[start + k] = even + odd;
                data[start + k + length / 2] = even - odd;
                factor *= root;
            }
        }
    }
}

}  // namespace

void fft(std::vector<std::complex<double>>& data) {
    if (!isPowerOfTwo(data.size())) {
        return;
    }
    bitReverse(data);
    butterflies(data);
}

}  // namespace masterizer::domain::dsp
