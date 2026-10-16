#include "masterizer/domain/analysis/BandFrames.h"

#include "masterizer/domain/analysis/BandMapping.h"
#include "masterizer/domain/analysis/SpectralFraming.h"
#include "masterizer/domain/common/OctaveBands.h"
#include "masterizer/domain/dsp/Fft.h"

#include <cmath>
#include <complex>
#include <cstddef>
#include <utility>
#include <vector>

namespace masterizer::domain::analysis {

std::vector<std::vector<double>> frameBandEnergies(const float* samples, int count,
                                                   double sampleRate, int fftSize, int hop) {
    std::vector<std::vector<double>> frames;
    if (samples == nullptr || sampleRate <= 0.0 || hop <= 0 || fftSize < 2 ||
        !isPowerOfTwo(fftSize) || count < fftSize) {
        return frames;
    }
    const std::vector<double> window = hannWindow(fftSize);
    const int bands = domain::bandCount();
    const double binHz = sampleRate / static_cast<double>(fftSize);
    std::vector<std::complex<double>> buffer(static_cast<std::size_t>(fftSize));
    for (int start = 0; start + fftSize <= count; start += hop) {
        for (int i = 0; i < fftSize; ++i) {
            const double value = window[static_cast<std::size_t>(i)] *
                                 static_cast<double>(samples[start + i]);
            buffer[static_cast<std::size_t>(i)] = std::complex<double>(value, 0.0);
        }
        dsp::fft(buffer);
        std::vector<double> energies(static_cast<std::size_t>(bands), 0.0);
        for (int bin = 0; bin <= fftSize / 2; ++bin) {
            const int band = bandIndexForFrequency(static_cast<double>(bin) * binHz);
            if (band >= 0) {
                const double magnitude = std::abs(buffer[static_cast<std::size_t>(bin)]);
                energies[static_cast<std::size_t>(band)] += magnitude * magnitude;
            }
        }
        frames.push_back(std::move(energies));
    }
    return frames;
}

}  // namespace masterizer::domain::analysis
