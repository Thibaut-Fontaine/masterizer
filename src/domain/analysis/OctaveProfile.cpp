#include "masterizer/domain/analysis/OctaveProfile.h"

#include "masterizer/domain/analysis/BandMapping.h"
#include "masterizer/domain/analysis/PowerSpectrum.h"
#include "masterizer/domain/common/OctaveBands.h"

#include <cstddef>

namespace masterizer::domain::analysis {

std::vector<double> octaveBandEnergies(const float* samples, int count,
                                       double sampleRate, int fftSize, int hop) {
    std::vector<double> energies(static_cast<std::size_t>(domain::bandCount()), 0.0);
    if (sampleRate <= 0.0) {
        return energies;
    }
    const std::vector<double> power = averagePowerSpectrum(samples, count, fftSize, hop);
    if (power.empty()) {
        return energies;
    }
    const double binHz = sampleRate / static_cast<double>(fftSize);
    for (std::size_t bin = 0; bin < power.size(); ++bin) {
        const int band = bandIndexForFrequency(static_cast<double>(bin) * binHz);
        if (band >= 0) {
            energies[static_cast<std::size_t>(band)] += power[bin];
        }
    }
    return energies;
}

}  // namespace masterizer::domain::analysis
