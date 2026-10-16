#include "masterizer/domain/matching/ResonanceDetector.h"

#include <algorithm>
#include <cstddef>
#include <vector>
#include <utility>

namespace masterizer::domain::matching {
namespace {

double medianOfNeighbourhood(const std::vector<double>& power, std::size_t centre, int window) {
    std::vector<double> neighbours;
    const int begin = static_cast<int>(centre) - window;
    const int end = static_cast<int>(centre) + window;
    for (int bin = begin; bin <= end; ++bin) {
        if (bin < 0 || bin >= static_cast<int>(power.size()) || bin == static_cast<int>(centre)) {
            continue;
        }
        neighbours.push_back(power[static_cast<std::size_t>(bin)]);
    }
    if (neighbours.empty()) {
        return 0.0;
    }
    std::sort(neighbours.begin(), neighbours.end());
    return neighbours[neighbours.size() / 2u];
}

bool isLocalPeak(const std::vector<double>& power, std::size_t bin) {
    return power[bin] >= power[bin - 1] && power[bin] >= power[bin + 1];
}

}  // namespace

std::vector<dsp::EqBand> detectResonances(const std::vector<double>& powerSpectrum,
                                          double sampleRate, int fftSize,
                                          const ResonanceSettings& settings) {
    std::vector<dsp::EqBand> bands;
    if (powerSpectrum.size() < 3u || sampleRate <= 0.0 || fftSize <= 0 ||
        settings.medianWindow <= 0) {
        return bands;
    }
    const double binHz = sampleRate / static_cast<double>(fftSize);
    std::vector<std::pair<double, std::size_t>> candidates;
    for (std::size_t bin = 1; bin + 1 < powerSpectrum.size(); ++bin) {
        const double frequency = static_cast<double>(bin) * binHz;
        if (frequency < settings.minFrequencyHz) {
            continue;
        }
        const double median = medianOfNeighbourhood(powerSpectrum, bin, settings.medianWindow);
        if (median > 0.0 && powerSpectrum[bin] > settings.peakRatio * median &&
            isLocalPeak(powerSpectrum, bin)) {
            candidates.emplace_back(powerSpectrum[bin], bin);
        }
    }
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });
    const std::size_t count = std::min(static_cast<std::size_t>(settings.maxCount),
                                       candidates.size());
    for (std::size_t i = 0; i < count; ++i) {
        const double frequency = static_cast<double>(candidates[i].second) * binHz;
        bands.push_back({frequency, settings.q, settings.cutDb});
    }
    return bands;
}

}  // namespace masterizer::domain::matching
