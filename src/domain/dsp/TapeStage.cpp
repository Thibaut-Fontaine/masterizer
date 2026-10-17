#include "masterizer/domain/dsp/TapeStage.h"

#include "masterizer/domain/dsp/Biquad.h"
#include "masterizer/domain/dsp/Gain.h"

#include <algorithm>
#include <cmath>

namespace masterizer::domain::dsp {
namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kHeadBumpHz = 70.0;
constexpr double kRolloffHz = 14000.0;
constexpr double kShelfQ = 0.7071067811865476;

/// RBJ shelving filter (low shelf when `high` is false, high shelf otherwise).
BiquadCoefficients shelfCoefficients(double sampleRate, double frequencyHz, double q,
                                     double gainDb, bool high) {
    const double a = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * kPi * frequencyHz / sampleRate;
    const double cosW0 = std::cos(w0);
    const double alpha = std::sin(w0) / (2.0 * q);
    const double twoSqrtAAlpha = 2.0 * std::sqrt(a) * alpha;
    const double a0 = high ? (a + 1.0) - (a - 1.0) * cosW0 + twoSqrtAAlpha
                           : (a + 1.0) + (a - 1.0) * cosW0 + twoSqrtAAlpha;
    if (high) {
        return {a * ((a + 1.0) + (a - 1.0) * cosW0 + twoSqrtAAlpha) / a0,
                -2.0 * a * ((a - 1.0) + (a + 1.0) * cosW0) / a0,
                a * ((a + 1.0) + (a - 1.0) * cosW0 - twoSqrtAAlpha) / a0,
                2.0 * ((a - 1.0) - (a + 1.0) * cosW0) / a0,
                ((a + 1.0) - (a - 1.0) * cosW0 - twoSqrtAAlpha) / a0};
    }
    return {a * ((a + 1.0) - (a - 1.0) * cosW0 + twoSqrtAAlpha) / a0,
            2.0 * a * ((a - 1.0) - (a + 1.0) * cosW0) / a0,
            a * ((a + 1.0) - (a - 1.0) * cosW0 - twoSqrtAAlpha) / a0,
            -2.0 * ((a - 1.0) + (a + 1.0) * cosW0) / a0,
            ((a + 1.0) + (a - 1.0) * cosW0 - twoSqrtAAlpha) / a0};
}

}  // namespace

void applyTapeStage(float* samples, int count, double sampleRate, const TapeSettings& settings) {
    if (samples == nullptr || count <= 0 || sampleRate <= 0.0 || settings.mix <= 0.0) {
        return;
    }
    const BiquadCoefficients lowShelf =
        shelfCoefficients(sampleRate, kHeadBumpHz, kShelfQ, settings.headBumpDb, false);
    const BiquadCoefficients highShelf =
        shelfCoefficients(sampleRate, kRolloffHz, kShelfQ, settings.hfRolloffDb, true);
    BiquadState lowState;
    BiquadState highState;
    const double drive = dbToLinear(settings.driveDb);
    const double compensation = drive > 0.0 ? 1.0 / drive : 1.0;
    const double mix = std::clamp(settings.mix, 0.0, 1.0);
    for (int i = 0; i < count; ++i) {
        const double dry = static_cast<double>(samples[i]);
        double wet = processBiquad(lowShelf, lowState, dry);
        wet = processBiquad(highShelf, highState, wet);
        if (settings.driveDb > 0.0) {
            wet = std::tanh(drive * wet) * compensation;
        }
        samples[i] = static_cast<float>((1.0 - mix) * dry + mix * wet);
    }
}

}  // namespace masterizer::domain::dsp
