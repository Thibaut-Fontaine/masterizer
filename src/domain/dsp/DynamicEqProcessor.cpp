#include "masterizer/domain/dsp/DynamicEqProcessor.h"

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

DynamicEqProcessor::DynamicEqProcessor(double sampleRate,
                                       const DynamicEqBandSettings& settings)
    : settings_(settings) {
    valid_ = sampleRate > 0.0;
    if (!valid_) {
        return;
    }
    band_ = bandpassCoefficients(sampleRate, settings.frequencyHz, settings.q);
    attack_ = std::exp(-1.0 / (std::max(settings.attackSeconds, 1e-6) * sampleRate));
    release_ = std::exp(-1.0 / (std::max(settings.releaseSeconds, 1e-6) * sampleRate));
}

void DynamicEqProcessor::process(float* samples, int count) {
    if (!valid_ || samples == nullptr || count <= 0) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        const double input = static_cast<double>(samples[i]);
        const double bandValue = processBiquad(band_, state_, input);
        const double magnitude = std::fabs(bandValue);
        envelope_ = envelopeStep(envelope_, magnitude, magnitude > envelope_ ? attack_ : release_);
        const double levelDb = 20.0 * std::log10(std::max(envelope_, kFloor));
        double reductionDb = 0.0;
        if (levelDb > settings_.thresholdDb) {
            reductionDb = -std::min(settings_.maxCutDb, levelDb - settings_.thresholdDb);
        }
        samples[i] = static_cast<float>((input - bandValue) +
                                        bandValue * dbToLinear(reductionDb));
    }
}

}  // namespace masterizer::domain::dsp
