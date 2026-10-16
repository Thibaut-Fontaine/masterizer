#include "masterizer/domain/analysis/EnvelopeProfile.h"

#include "masterizer/domain/dsp/CrestFactor.h"
#include "masterizer/domain/dsp/Peak.h"
#include "masterizer/domain/dsp/Rms.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace masterizer::domain::analysis {
namespace {

constexpr double kEnvelopeSeconds = 0.01;
constexpr double kOnsetRatio = 2.0;
constexpr double kDensitySaturation = 8.0;
constexpr double kCrestFloorDb = 3.0;
constexpr double kCrestSaturationDb = 15.0;

std::vector<double> windowRms(const float* samples, int count, int windowSize) {
    std::vector<double> envelope;
    if (windowSize <= 0) {
        return envelope;
    }
    for (int start = 0; start + windowSize <= count; start += windowSize) {
        envelope.push_back(dsp::rms(samples + start, windowSize));
    }
    return envelope;
}

int countOnsets(const std::vector<double>& envelope) {
    int onsets = 0;
    for (std::size_t i = 1; i < envelope.size(); ++i) {
        if (envelope[i] > kOnsetRatio * envelope[i - 1] && envelope[i] > 1e-5) {
            ++onsets;
        }
    }
    return onsets;
}

}  // namespace

EnvelopeProfile analyzeEnvelope(const float* samples, int count, double sampleRate) {
    EnvelopeProfile profile;
    if (samples == nullptr || count <= 0 || sampleRate <= 0.0) {
        return profile;
    }
    profile.peak = dsp::peakAbs(samples, count);
    profile.rms = dsp::rms(samples, count);
    profile.crestDb = dsp::crestFactorDb(samples, count);

    const int windowSize = static_cast<int>(std::lround(kEnvelopeSeconds * sampleRate));
    const std::vector<double> envelope = windowRms(samples, count, windowSize);
    const int onsets = countOnsets(envelope);
    const double seconds = static_cast<double>(count) / sampleRate;
    profile.transientDensity = seconds > 0.0 ? onsets / seconds : 0.0;

    const double densityTerm = std::min(1.0, profile.transientDensity / kDensitySaturation);
    const double crestTerm = std::min(
        1.0, std::max(0.0, profile.crestDb - kCrestFloorDb) / kCrestSaturationDb);
    profile.percussiveProminence = 0.5 * densityTerm + 0.5 * crestTerm;
    return profile;
}

}  // namespace masterizer::domain::analysis
