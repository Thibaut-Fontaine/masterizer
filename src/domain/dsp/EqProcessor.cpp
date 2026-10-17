#include "masterizer/domain/dsp/EqProcessor.h"

#include "masterizer/domain/dsp/ShelfDesign.h"

#include <cstddef>
#include <vector>

namespace masterizer::domain::dsp {
namespace {

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

EqProcessor::EqProcessor(double sampleRate, const std::vector<EqBand>& bands) {
    if (sampleRate <= 0.0) {
        return;
    }
    coefficients_.reserve(bands.size());
    states_.resize(bands.size());
    for (const EqBand& band : bands) {
        coefficients_.push_back(coefficientsFor(band, sampleRate));
    }
}

void EqProcessor::process(float* samples, int count) {
    if (samples == nullptr || count <= 0) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        double value = static_cast<double>(samples[i]);
        for (std::size_t band = 0; band < coefficients_.size(); ++band) {
            value = processBiquad(coefficients_[band], states_[band], value);
        }
        samples[i] = static_cast<float>(value);
    }
}

}  // namespace masterizer::domain::dsp
