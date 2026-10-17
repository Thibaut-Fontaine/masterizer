#include "masterizer/domain/matching/FineEqDesigner.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace masterizer::domain::matching {
namespace {

constexpr double kThirdOctave = 1.2599210498948732;  // 2^(1/3)
constexpr double kSixthOctave = 1.122462048309373;   // 2^(1/6)

double totalEnergy(const std::vector<double>& spectrum) {
    double total = 0.0;
    for (double magnitude : spectrum) {
        total += magnitude * magnitude;
    }
    return total;
}

double bandEnergy(const std::vector<double>& spectrum, double binHz, double low, double high) {
    double sum = 0.0;
    for (std::size_t bin = 0; bin < spectrum.size(); ++bin) {
        const double frequency = static_cast<double>(bin) * binHz;
        if (frequency >= low && frequency < high) {
            sum += spectrum[bin] * spectrum[bin];
        }
    }
    return sum;
}

}  // namespace

std::vector<dsp::EqBand> designFineEq(const std::vector<double>& sourceSpectrum,
                                      const std::vector<double>& referenceSpectrum,
                                      double sampleRate, int fftSize,
                                      const CorrectionBounds& bounds,
                                      const FineEqSettings& settings) {
    std::vector<dsp::EqBand> bands;
    if (sourceSpectrum.empty() || referenceSpectrum.empty() || sampleRate <= 0.0 ||
        fftSize <= 0) {
        return bands;
    }
    const double sourceTotal = totalEnergy(sourceSpectrum);
    const double referenceTotal = totalEnergy(referenceSpectrum);
    if (sourceTotal <= 0.0 || referenceTotal <= 0.0) {
        return bands;
    }
    const double binHz = sampleRate / static_cast<double>(fftSize);
    struct Candidate {
        double frequency;
        double gainDb;
    };
    std::vector<Candidate> candidates;
    for (double centre = settings.minHz; centre <= settings.maxHz; centre *= kThirdOctave) {
        const double low = centre / kSixthOctave;
        const double high = centre * kSixthOctave;
        const double sourceLevel = bandEnergy(sourceSpectrum, binHz, low, high) / sourceTotal;
        const double referenceLevel =
            bandEnergy(referenceSpectrum, binHz, low, high) / referenceTotal;
        if (sourceLevel <= 0.0 || referenceLevel <= 0.0) {
            continue;
        }
        const double differenceDb = 10.0 * std::log10(sourceLevel / referenceLevel);
        if (std::fabs(differenceDb) < settings.minDifferenceDb) {
            continue;
        }
        const double gain = std::clamp(-differenceDb, -bounds.maxEqCutDb, bounds.maxEqBoostDb);
        candidates.push_back({centre, gain});
    }
    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        return std::fabs(a.gainDb) > std::fabs(b.gainDb);
    });
    const std::size_t count =
        std::min(static_cast<std::size_t>(settings.maxFilters), candidates.size());
    for (std::size_t i = 0; i < count; ++i) {
        bands.push_back({candidates[i].frequency, settings.q, candidates[i].gainDb,
                         dsp::EqShape::Peaking});
    }
    return bands;
}

}  // namespace masterizer::domain::matching
