#include "masterizer/domain/dsp/TapeStage.h"

#include "masterizer/domain/dsp/Gain.h"
#include "masterizer/domain/dsp/ShelfDesign.h"

#include <algorithm>
#include <cmath>

namespace masterizer::domain::dsp {
namespace {

constexpr double kHeadBumpHz = 70.0;
constexpr double kRolloffHz = 14000.0;
constexpr double kShelfQ = 0.7071067811865476;

}  // namespace

void applyTapeStage(float* samples, int count, double sampleRate, const TapeSettings& settings) {
    if (samples == nullptr || count <= 0 || sampleRate <= 0.0 || settings.mix <= 0.0) {
        return;
    }
    const BiquadCoefficients lowShelf =
        lowShelfCoefficients(sampleRate, kHeadBumpHz, kShelfQ, settings.headBumpDb);
    const BiquadCoefficients highShelf =
        highShelfCoefficients(sampleRate, kRolloffHz, kShelfQ, settings.hfRolloffDb);
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
