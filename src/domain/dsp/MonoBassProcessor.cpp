#include "masterizer/domain/dsp/MonoBassProcessor.h"

#include "masterizer/domain/dsp/FilterDesign.h"

namespace masterizer::domain::dsp {
namespace {

constexpr double kButterworthQ = 0.7071067811865476;

}  // namespace

MonoBassProcessor::MonoBassProcessor(double sampleRate, const MonoBassSettings& settings)
    : settings_(settings) {
    valid_ = sampleRate > 0.0;
    if (valid_) {
        highPass_ = highpassCoefficients(sampleRate, settings.transitionHz, kButterworthQ);
    }
}

void MonoBassProcessor::process(float* left, float* right, int count) {
    if (!valid_ || left == nullptr || right == nullptr || count <= 0) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        const double mid = 0.5 * (static_cast<double>(left[i]) + static_cast<double>(right[i]));
        const double side = 0.5 * (static_cast<double>(left[i]) - static_cast<double>(right[i]));
        double sideOut = processBiquad(highPass_, firstStage_, side);
        if (settings_.strictMono) {
            sideOut = processBiquad(highPass_, secondStage_, sideOut);
        }
        left[i] = static_cast<float>(mid + sideOut);
        right[i] = static_cast<float>(mid - sideOut);
    }
}

}  // namespace masterizer::domain::dsp
