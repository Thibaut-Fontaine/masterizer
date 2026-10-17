#include "masterizer/domain/analysis/StereoProfile.h"

#include "masterizer/domain/analysis/BandMapping.h"
#include "masterizer/domain/analysis/OctaveProfile.h"
#include "masterizer/domain/analysis/SpectralFraming.h"
#include "masterizer/domain/audio/AudioBufferOps.h"
#include "masterizer/domain/common/OctaveBands.h"
#include "masterizer/domain/dsp/Correlation.h"
#include "masterizer/domain/dsp/Fft.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <vector>
#include <vector>

namespace masterizer::domain::analysis {
namespace {

std::vector<float> midChannel(const std::vector<float>& left, const std::vector<float>& right) {
    std::vector<float> mid(left.size());
    for (std::size_t i = 0; i < left.size(); ++i) {
        mid[i] = 0.5f * (left[i] + right[i]);
    }
    return mid;
}

std::vector<float> sideChannel(const std::vector<float>& left, const std::vector<float>& right) {
    std::vector<float> side(left.size());
    for (std::size_t i = 0; i < left.size(); ++i) {
        side[i] = 0.5f * (left[i] - right[i]);
    }
    return side;
}

double total(const std::vector<double>& values) {
    double sum = 0.0;
    for (double value : values) {
        sum += value;
    }
    return sum;
}

/// True per-band L/R correlation from the cross-spectrum:
/// sum Re(L*conj(R)) / sqrt(sum|L|^2 sum|R|^2).
std::vector<double> bandCorrelation(const std::vector<float>& left,
                                    const std::vector<float>& right, double sampleRate,
                                    int fftSize, int hop) {
    std::vector<double> result(static_cast<std::size_t>(domain::bandCount()), 0.0);
    const std::size_t frames = std::min(left.size(), right.size());
    if (frames == 0 || sampleRate <= 0.0 || hop <= 0 || fftSize < 2 ||
        !isPowerOfTwo(fftSize) || frames < static_cast<std::size_t>(fftSize)) {
        return result;
    }
    const std::vector<double> window = hannWindow(fftSize);
    const double binHz = sampleRate / static_cast<double>(fftSize);
    std::vector<double> sumLR(result.size(), 0.0);
    std::vector<double> sumLL(result.size(), 0.0);
    std::vector<double> sumRR(result.size(), 0.0);
    std::vector<std::complex<double>> leftBuffer(static_cast<std::size_t>(fftSize));
    std::vector<std::complex<double>> rightBuffer(static_cast<std::size_t>(fftSize));
    for (std::size_t start = 0; start + static_cast<std::size_t>(fftSize) <= frames;
         start += static_cast<std::size_t>(hop)) {
        for (int i = 0; i < fftSize; ++i) {
            const double w = window[static_cast<std::size_t>(i)];
            leftBuffer[static_cast<std::size_t>(i)] =
                std::complex<double>(w * left[start + static_cast<std::size_t>(i)], 0.0);
            rightBuffer[static_cast<std::size_t>(i)] =
                std::complex<double>(w * right[start + static_cast<std::size_t>(i)], 0.0);
        }
        dsp::fft(leftBuffer);
        dsp::fft(rightBuffer);
        for (int bin = 0; bin <= fftSize / 2; ++bin) {
            const int band = bandIndexForFrequency(static_cast<double>(bin) * binHz);
            if (band < 0) {
                continue;
            }
            const std::size_t k = static_cast<std::size_t>(band);
            const std::complex<double> l = leftBuffer[static_cast<std::size_t>(bin)];
            const std::complex<double> r = rightBuffer[static_cast<std::size_t>(bin)];
            sumLR[k] += (l * std::conj(r)).real();
            sumLL[k] += std::norm(l);
            sumRR[k] += std::norm(r);
        }
    }
    for (std::size_t band = 0; band < result.size(); ++band) {
        const double denominator = std::sqrt(sumLL[band] * sumRR[band]);
        result[band] = denominator > 0.0 ? std::clamp(sumLR[band] / denominator, -1.0, 1.0)
                                         : 0.0;
    }
    return result;
}

}  // namespace

StereoProfile analyzeStereo(const audio::AudioBuffer& buffer, int fftSize, int hop) {
    StereoProfile profile;
    if (audio::channelCount(buffer) < 2 || audio::frameCount(buffer) <= 0) {
        return profile;
    }
    const std::vector<float>& left = buffer.channels[0];
    const std::vector<float>& right = buffer.channels[1];
    const int frames = audio::frameCount(buffer);

    const std::vector<float> mid = midChannel(left, right);
    const std::vector<float> side = sideChannel(left, right);
    profile.midEnergy = octaveBandEnergies(mid.data(), frames, buffer.sampleRate, fftSize, hop);
    profile.sideEnergy = octaveBandEnergies(side.data(), frames, buffer.sampleRate, fftSize, hop);
    profile.bandCorrelation = bandCorrelation(left, right, buffer.sampleRate, fftSize, hop);
    profile.correlation = dsp::correlation(left.data(), right.data(), frames);

    const double midTotal = total(profile.midEnergy);
    const double sideTotal = total(profile.sideEnergy);
    const double combined = midTotal + sideTotal;
    profile.width = combined > 0.0 ? sideTotal / combined : 0.0;
    return profile;
}

}  // namespace masterizer::domain::analysis
