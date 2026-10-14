#include "masterizer/domain/analysis/BandDynamics.h"

#include "masterizer/domain/analysis/BandFrames.h"
#include "masterizer/domain/common/OctaveBands.h"

#include <cmath>
#include <cstddef>
#include <vector>

namespace masterizer::domain::analysis {
namespace {

double coefficientOfVariation(const std::vector<double>& values) {
    const std::size_t size = values.size();
    if (size == 0) {
        return 0.0;
    }
    double mean = 0.0;
    for (double value : values) {
        mean += value;
    }
    mean /= static_cast<double>(size);
    if (mean <= 0.0) {
        return 0.0;
    }
    double variance = 0.0;
    for (double value : values) {
        const double delta = value - mean;
        variance += delta * delta;
    }
    variance /= static_cast<double>(size);
    return std::sqrt(variance) / mean;
}

}  // namespace

std::vector<double> bandVariation(const float* samples, int count, double sampleRate,
                                  int fftSize, int hop) {
    const int bands = domain::bandCount();
    std::vector<double> variation(static_cast<std::size_t>(bands), 0.0);
    const std::vector<std::vector<double>> frames =
        frameBandEnergies(samples, count, sampleRate, fftSize, hop);
    if (frames.empty()) {
        return variation;
    }
    for (int band = 0; band < bands; ++band) {
        std::vector<double> series;
        series.reserve(frames.size());
        for (const std::vector<double>& frame : frames) {
            series.push_back(frame[static_cast<std::size_t>(band)]);
        }
        variation[static_cast<std::size_t>(band)] = coefficientOfVariation(series);
    }
    return variation;
}

}  // namespace masterizer::domain::analysis
