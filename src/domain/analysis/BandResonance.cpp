#include "masterizer/domain/analysis/BandResonance.h"

#include "masterizer/domain/analysis/BandMapping.h"
#include "masterizer/domain/analysis/BandFrames.h"
#include "masterizer/domain/analysis/PowerSpectrum.h"
#include "masterizer/domain/common/OctaveBands.h"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace masterizer::domain::analysis {
namespace {

constexpr double kPersistenceRatio = 1.5;

double bandPersistence(const std::vector<std::vector<double>>& frames, int band) {
    if (frames.empty()) {
        return 0.0;
    }
    double mean = 0.0;
    for (const std::vector<double>& frame : frames) {
        mean += frame[static_cast<std::size_t>(band)];
    }
    mean /= static_cast<double>(frames.size());
    if (mean <= 0.0) {
        return 0.0;
    }
    int above = 0;
    for (const std::vector<double>& frame : frames) {
        if (frame[static_cast<std::size_t>(band)] > kPersistenceRatio * mean) {
            ++above;
        }
    }
    return static_cast<double>(above) / static_cast<double>(frames.size());
}

double bandPeakiness(const std::vector<double>& power, double binHz, int band) {
    std::vector<double> bins;
    for (std::size_t bin = 0; bin < power.size(); ++bin) {
        if (bandIndexForFrequency(static_cast<double>(bin) * binHz) == band) {
            bins.push_back(power[bin]);
        }
    }
    if (bins.size() < 2u) {
        return 0.0;
    }
    std::sort(bins.begin(), bins.end());
    const double median = bins[bins.size() / 2u];
    return median > 0.0 ? bins.back() / median : 0.0;
}

}  // namespace

BandResonanceProfile analyzeBandResonance(const float* samples, int count, double sampleRate,
                                          int fftSize, int hop) {
    const int bands = domain::bandCount();
    BandResonanceProfile profile;
    profile.persistence.assign(static_cast<std::size_t>(bands), 0.0);
    profile.peakiness.assign(static_cast<std::size_t>(bands), 0.0);
    if (sampleRate <= 0.0) {
        return profile;
    }
    const std::vector<std::vector<double>> frames =
        frameBandEnergies(samples, count, sampleRate, fftSize, hop);
    const std::vector<double> power = averagePowerSpectrum(samples, count, fftSize, hop);
    const double binHz = sampleRate / static_cast<double>(fftSize);
    for (int band = 0; band < bands; ++band) {
        profile.persistence[static_cast<std::size_t>(band)] = bandPersistence(frames, band);
        profile.peakiness[static_cast<std::size_t>(band)] = bandPeakiness(power, binHz, band);
    }
    return profile;
}

}  // namespace masterizer::domain::analysis
