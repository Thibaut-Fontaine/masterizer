#include "masterizer/domain/dsp/Fft.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <cmath>
#include <complex>
#include <cstddef>
#include <vector>

using masterizer::domain::dsp::fft;

namespace {

constexpr double kPi = 3.14159265358979323846;

}  // namespace

TEST_CASE(fft_of_impulse_is_flat) {
    std::vector<std::complex<double>> data(8);
    data[0] = std::complex<double>(1.0, 0.0);
    fft(data);
    for (const auto& value : data) {
        CHECK_NEAR(std::abs(value), 1.0, 1e-9);
    }
}

TEST_CASE(fft_of_constant_is_dc_only) {
    std::vector<std::complex<double>> data(8, std::complex<double>(1.0, 0.0));
    fft(data);
    CHECK_NEAR(std::abs(data[0]), 8.0, 1e-9);
    for (std::size_t bin = 1; bin < data.size(); ++bin) {
        CHECK_NEAR(std::abs(data[bin]), 0.0, 1e-9);
    }
}

TEST_CASE(fft_of_single_bin_cosine_splits_into_conjugates) {
    std::vector<std::complex<double>> data(8);
    for (int n = 0; n < 8; ++n) {
        data[static_cast<std::size_t>(n)] =
            std::complex<double>(std::cos(2.0 * kPi * n / 8.0), 0.0);
    }
    fft(data);
    CHECK_NEAR(std::abs(data[1]), 4.0, 1e-9);
    CHECK_NEAR(std::abs(data[7]), 4.0, 1e-9);
}
