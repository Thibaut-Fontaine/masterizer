#include "masterizer/domain/analysis/BandTransient.h"

#include "masterizer/domain/analysis/BandFrames.h"
#include "masterizer/domain/common/OctaveBands.h"

#include <cstddef>
#include <vector>

namespace masterizer::domain::analysis {
namespace {

constexpr double kOnsetRatio = 2.0;
constexpr double kOnsetFloor = 1e-6;

double bandDensity(const std::vector<std::vector<double>>& frames, int band,
                   double seconds) {
    int onsets = 0;
    for (std::size_t i = 1; i < frames.size(); ++i) {
        const double previous = frames[i - 1][static_cast<std::size_t>(band)];
        const double current = frames[i][static_cast<std::size_t>(band)];
        if (current > kOnsetRatio * previous && current > kOnsetFloor) {
            ++onsets;
        }
    }
    return seconds > 0.0 ? static_cast<double>(onsets) / seconds : 0.0;
}

double bandCrest(const std::vector<std::vector<double>>& frames, int band) {
    if (frames.empty()) {
        return 0.0;
    }
    double maximum = 0.0;
    double sum = 0.0;
    for (const std::vector<double>& frame : frames) {
        const double value = frame[static_cast<std::size_t>(band)];
        maximum = value > maximum ? value : maximum;
        sum += value;
    }
    const double mean = sum / static_cast<double>(frames.size());
    return mean > 0.0 ? maximum / mean : 0.0;
}

}  // namespace

BandTransientProfile analyzeBandTransient(const float* samples, int count, double sampleRate,
                                          int fftSize, int hop) {
    const int bands = domain::bandCount();
    BandTransientProfile profile;
    profile.density.assign(static_cast<std::size_t>(bands), 0.0);
    profile.crest.assign(static_cast<std::size_t>(bands), 0.0);
    const std::vector<std::vector<double>> frames =
        frameBandEnergies(samples, count, sampleRate, fftSize, hop);
    if (frames.empty() || sampleRate <= 0.0) {
        return profile;
    }
    const double seconds = static_cast<double>(count) / sampleRate;
    for (int band = 0; band < bands; ++band) {
        profile.density[static_cast<std::size_t>(band)] = bandDensity(frames, band, seconds);
        profile.crest[static_cast<std::size_t>(band)] = bandCrest(frames, band);
    }
    return profile;
}

}  // namespace masterizer::domain::analysis
