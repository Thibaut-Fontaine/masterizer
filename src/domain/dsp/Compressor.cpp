#include "masterizer/domain/dsp/Compressor.h"

#include "masterizer/domain/dsp/Gain.h"

#include <algorithm>
#include <cmath>

namespace masterizer::domain::dsp {
namespace {

constexpr double kFloor = 1e-12;

double gainReductionDb(double levelDb, const CompressorSettings& settings) {
    const double slope = 1.0 - 1.0 / std::max(settings.ratio, 1.0);
    const double overshoot = levelDb - settings.thresholdDb;
    const double knee = settings.kneeDb;
    if (knee > 0.0 && overshoot < knee * 0.5) {
        if (overshoot <= -knee * 0.5) {
            return 0.0;
        }
        const double x = overshoot + knee * 0.5;
        return -slope * x * x / (2.0 * knee);
    }
    return -slope * std::max(0.0, overshoot);
}

void updateEnvelope(CompressorState& state, double magnitude, double attack, double release) {
    const double coefficient = magnitude > state.envelope ? attack : release;
    state.envelope = coefficient * state.envelope + (1.0 - coefficient) * magnitude;
}

}  // namespace

double compressSample(double input, const CompressorSettings& settings, double sampleRate,
                      CompressorState& state) {
    const double attack = std::exp(-1.0 / (std::max(settings.attackSeconds, 1e-6) * sampleRate));
    const double release =
        std::exp(-1.0 / (std::max(settings.releaseSeconds, 1e-6) * sampleRate));
    updateEnvelope(state, std::fabs(input), attack, release);
    const double levelDb = 20.0 * std::log10(std::max(state.envelope, kFloor));
    const double gainDb = gainReductionDb(levelDb, settings) + settings.makeupDb;
    return input * dbToLinear(gainDb);
}

void applyCompressor(float* samples, int count, double sampleRate,
                     const CompressorSettings& settings) {
    if (samples == nullptr || count <= 0 || sampleRate <= 0.0) {
        return;
    }
    CompressorState state;
    for (int i = 0; i < count; ++i) {
        samples[i] = static_cast<float>(
            compressSample(static_cast<double>(samples[i]), settings, sampleRate, state));
    }
}

}  // namespace masterizer::domain::dsp
