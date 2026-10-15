#include "masterizer/domain/dsp/MonoBass.h"

#include "masterizer/domain/dsp/Biquad.h"
#include "masterizer/domain/dsp/FilterDesign.h"

namespace masterizer::domain::dsp {
namespace {

constexpr double kButterworthQ = 0.7071067811865476;

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
