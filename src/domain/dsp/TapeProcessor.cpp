#include "masterizer/domain/dsp/TapeProcessor.h"

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

TapeProcessor::TapeProcessor(double sampleRate, const TapeSettings& settings) {
    valid_ = sampleRate > 0.0 && settings.mix > 0.0;
    if (!valid_) {
        return;
    }
    lowShelf_ = lowShelfCoefficients(sampleRate, kHeadBumpHz, kShelfQ, settings.headBumpDb);
    highShelf_ = highShelfCoefficients(sampleRate, kRolloffHz, kShelfQ, settings.hfRolloffDb);
    drive_ = dbToLinear(settings.driveDb);
    compensation_ = drive_ > 0.0 ? 1.0 / drive_ : 1.0;
    mix_ = std::clamp(settings.mix, 0.0, 1.0);
}

void TapeProcessor::process(float* samples, int count) {
    if (!valid_ || samples == nullptr || count <= 0) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        const double dry = static_cast<double>(samples[i]);
        double wet = processBiquad(lowShelf_, lowState_, dry);
        wet = processBiquad(highShelf_, highState_, wet);
        if (drive_ > 0.0) {
            wet = std::tanh(drive_ * wet) * compensation_;
        }
        samples[i] = static_cast<float>((1.0 - mix_) * dry + mix_ * wet);
    }
}

}  // namespace masterizer::domain::dsp
