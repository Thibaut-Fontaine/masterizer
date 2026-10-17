#include "masterizer/domain/dsp/ParametricEq.h"

#include "masterizer/domain/dsp/ShelfDesign.h"

#include <cmath>
#include <vector>

namespace masterizer::domain::dsp {
namespace {

constexpr double kPi = 3.14159265358979323846;

BiquadCoefficients coefficientsFor(const EqBand& band, double sampleRate) {
    switch (band.shape) {
        case EqShape::LowShelf:
            return lowShelfCoefficients(sampleRate, band.frequencyHz, band.q, band.gainDb);
        case EqShape::HighShelf:
            return highShelfCoefficients(sampleRate, band.frequencyHz, band.q, band.gainDb);
        case EqShape::Peaking:
            break;
    }
    return peakingEqCoefficients(sampleRate, band.frequencyHz, band.q, band.gainDb);
}

}  // namespace

BiquadCoefficients peakingEqCoefficients(double sampleRate, double frequencyHz, double q,
                                         double gainDb) {
    const double a = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * kPi * frequencyHz / sampleRate;
    const double alpha = std::sin(w0) / (2.0 * q);
    const double cosW0 = std::cos(w0);
    const double a0 = 1.0 + alpha / a;
    return {(1.0 + alpha * a) / a0,
            (-2.0 * cosW0) / a0,
            (1.0 - alpha * a) / a0,
            (-2.0 * cosW0) / a0,
            (1.0 - alpha / a) / a0};
}

void applyEqBands(float* samples, int count, double sampleRate,
                  const std::vector<EqBand>& bands) {
    for (const EqBand& band : bands) {
        const BiquadCoefficients coefficients = coefficientsFor(band, sampleRate);
        BiquadState state;
        for (int i = 0; i < count; ++i) {
            samples[i] = static_cast<float>(
                processBiquad(coefficients, state, static_cast<double>(samples[i])));
        }
    }
}

}  // namespace masterizer::domain::dsp
