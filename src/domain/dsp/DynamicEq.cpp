#include "masterizer/domain/dsp/DynamicEq.h"

#include "masterizer/domain/dsp/FilterDesign.h"
#include "masterizer/domain/dsp/Gain.h"

#include <algorithm>
#include <cmath>

namespace masterizer::domain::dsp {
namespace {

constexpr double kFloor = 1e-12;

double envelopeStep(double current, double target, double coefficient) {
    return coefficient * current + (1.0 - coefficient) * target;
}

}  // namespace

void applyDynamicEqBand(float* samples, int count, double sampleRate,
                        const DynamicEqBandSettings& settings) {
    if (samples == nullptr || count <= 0 || sampleRate <= 0.0) {
        return;
    }
    const BiquadCoefficients band =
        bandpassCoefficients(sampleRate, settings.frequencyHz, settings.q);
    const double attack =
        std::exp(-1.0 / (std::max(settings.attackSeconds, 1e-6) * sampleRate));
    const double release =
        std::exp(-1.0 / (std::max(settings.releaseSeconds, 1e-6) * sampleRate));
    BiquadState state;
    double envelope = 0.0;
    for (int i = 0; i < count; ++i) {
        const double input = static_cast<double>(samples[i]);
        const double bandValue = processBiquad(band, state, input);
        const double magnitude = std::fabs(bandValue);
        envelope = envelopeStep(envelope, magnitude, magnitude > envelope ? attack : release);
        const double levelDb = 20.0 * std::log10(std::max(envelope, kFloor));
        double reductionDb = 0.0;
        if (levelDb > settings.thresholdDb) {
            reductionDb = -std::min(settings.maxCutDb, levelDb - settings.thresholdDb);
        }
        samples[i] = static_cast<float>((input - bandValue) +
                                        bandValue * dbToLinear(reductionDb));
    }
}

}  // namespace masterizer::domain::dsp
