#include "masterizer/domain/dsp/MultibandCompressor.h"

#include "masterizer/domain/dsp/FilterDesign.h"

#include <cstddef>
#include <vector>

namespace masterizer::domain::dsp {
namespace {

constexpr double kButterworthQ = 0.7071067811865476;

/// LR4 split: two cascaded Butterworth sections per band.
void splitBands(const float* samples, int count, double sampleRate, double crossoverHz,
                std::vector<float>& low, std::vector<float>& high) {
    const BiquadCoefficients lowPass =
        lowpassCoefficients(sampleRate, crossoverHz, kButterworthQ);
    const BiquadCoefficients highPass =
        highpassCoefficients(sampleRate, crossoverHz, kButterworthQ);
    BiquadState lowStage1;
    BiquadState lowStage2;
    BiquadState highStage1;
    BiquadState highStage2;
    for (int i = 0; i < count; ++i) {
        const double value = static_cast<double>(samples[i]);
        double lowValue = processBiquad(lowPass, lowStage1, value);
        lowValue = processBiquad(lowPass, lowStage2, lowValue);
        double highValue = processBiquad(highPass, highStage1, value);
        highValue = processBiquad(highPass, highStage2, highValue);
        low[static_cast<std::size_t>(i)] = static_cast<float>(lowValue);
        high[static_cast<std::size_t>(i)] = static_cast<float>(highValue);
    }
}

}  // namespace

void applyMultibandCompressor(float* samples, int count, double sampleRate,
                              const MultibandCompressorSettings& settings) {
    if (samples == nullptr || count <= 0 || sampleRate <= 0.0) {
        return;
    }
    std::vector<float> low(static_cast<std::size_t>(count), 0.0f);
    std::vector<float> high(static_cast<std::size_t>(count), 0.0f);
    splitBands(samples, count, sampleRate, settings.crossoverHz, low, high);
    applyCompressor(low.data(), count, sampleRate, settings.low);
    applyCompressor(high.data(), count, sampleRate, settings.high);
    for (int i = 0; i < count; ++i) {
        samples[i] = low[static_cast<std::size_t>(i)] + high[static_cast<std::size_t>(i)];
    }
}

}  // namespace masterizer::domain::dsp
