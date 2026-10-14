#include "masterizer/domain/analysis/Stft.h"

#include "masterizer/domain/dsp/Fft.h"

#include <cmath>
#include <complex>
#include <cstddef>

namespace masterizer::domain::analysis {
namespace {

constexpr double kTwoPi = 2.0 * 3.14159265358979323846;

bool isPowerOfTwo(int value) {
    return value > 0 && (value & (value - 1)) == 0;
}

std::vector<double> hannWindow(int size) {
    std::vector<double> window(static_cast<std::size_t>(size), 1.0);
    for (int i = 0; i < size; ++i) {
        window[static_cast<std::size_t>(i)] =
            0.5 - 0.5 * std::cos(kTwoPi * i / static_cast<double>(size - 1));
    }
    return window;
}

void accumulateFrame(const float* samples, const std::vector<double>& window, int start,
                     std::vector<std::complex<double>>& buffer, std::vector<double>& sums) {
    const int size = static_cast<int>(window.size());
    for (int i = 0; i < size; ++i) {
        const double value = window[static_cast<std::size_t>(i)] *
                             static_cast<double>(samples[start + i]);
        buffer[static_cast<std::size_t>(i)] = std::complex<double>(value, 0.0);
    }
    dsp::fft(buffer);
    for (int bin = 0; bin <= size / 2; ++bin) {
        sums[static_cast<std::size_t>(bin)] +=
            std::abs(buffer[static_cast<std::size_t>(bin)]);
    }
}

}  // namespace

std::vector<double> averageMagnitudeSpectrum(const float* samples, int count,
                                             int fftSize, int hop) {
    if (samples == nullptr || hop <= 0 || fftSize < 2 || !isPowerOfTwo(fftSize) ||
        count < fftSize) {
        return {};
    }
    const std::vector<double> window = hannWindow(fftSize);
    std::vector<double> sums(static_cast<std::size_t>(fftSize / 2 + 1), 0.0);
    std::vector<std::complex<double>> buffer(static_cast<std::size_t>(fftSize));
    int frames = 0;
    for (int start = 0; start + fftSize <= count; start += hop) {
        accumulateFrame(samples, window, start, buffer, sums);
        ++frames;
    }
    for (double& value : sums) {
        value /= frames;
    }
    return sums;
}

}  // namespace masterizer::domain::analysis
