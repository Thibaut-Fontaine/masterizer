#include "masterizer/domain/dsp/MonoBass.h"

#include "masterizer/domain/dsp/Biquad.h"

#include <cmath>

namespace masterizer::domain::dsp {
namespace {

constexpr double kTwoPi = 2.0 * 3.14159265358979323846;
constexpr double kButterworthQ = 0.7071067811865476;

/// RBJ 2nd-order high-pass (normalised so a0 = 1).
BiquadCoefficients highpassCoefficients(double sampleRate, double frequencyHz, double q) {
    const double w0 = kTwoPi * frequencyHz / sampleRate;
    const double cosW0 = std::cos(w0);
    const double alpha = std::sin(w0) / (2.0 * q);
    const double a0 = 1.0 + alpha;
    return {(1.0 + cosW0) / 2.0 / a0,
            -(1.0 + cosW0) / a0,
            (1.0 + cosW0) / 2.0 / a0,
            (-2.0 * cosW0) / a0,
            (1.0 - alpha) / a0};
}

}  // namespace

void applyMonoBass(float* left, float* right, int count, double sampleRate,
                   const MonoBassSettings& settings) {
    if (left == nullptr || right == nullptr || count <= 0 || sampleRate <= 0.0) {
        return;
    }
    // High-pass the Side so its low-frequency content is removed; the remaining
    // Mid keeps the low end centred. Strict mode cascades a second stage.
    const BiquadCoefficients highPass =
        highpassCoefficients(sampleRate, settings.transitionHz, kButterworthQ);
    BiquadState firstStage;
    BiquadState secondStage;
    for (int i = 0; i < count; ++i) {
        const double mid = 0.5 * (static_cast<double>(left[i]) + static_cast<double>(right[i]));
        const double side = 0.5 * (static_cast<double>(left[i]) - static_cast<double>(right[i]));
        double sideOut = processBiquad(highPass, firstStage, side);
        if (settings.strictMono) {
            sideOut = processBiquad(highPass, secondStage, sideOut);
        }
        left[i] = static_cast<float>(mid + sideOut);
        right[i] = static_cast<float>(mid - sideOut);
    }
}

}  // namespace masterizer::domain::dsp
