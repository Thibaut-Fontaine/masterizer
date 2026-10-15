#include "masterizer/domain/dsp/FilterDesign.h"

#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::domain::dsp::BiquadCoefficients;
using masterizer::domain::dsp::BiquadState;
using masterizer::domain::dsp::highpassCoefficients;
using masterizer::domain::dsp::lowpassCoefficients;
using masterizer::domain::dsp::processBiquad;
using masterizer::domain::dsp::rms;
using masterizer::test::sine;

namespace {

double filteredRms(std::vector<float> data, const BiquadCoefficients& coefficients) {
    BiquadState state;
    for (float& sample : data) {
        sample = static_cast<float>(processBiquad(coefficients, state, sample));
    }
    return rms(data.data(), static_cast<int>(data.size()));
}

}  // namespace

TEST_CASE(lowpass_passes_low_and_attenuates_high) {
    const auto coefficients = lowpassCoefficients(48000.0, 1000.0, 0.70710678);
    const double low = filteredRms(sine(48000, 100.0, 48000.0, 1.0f), coefficients);
    const double high = filteredRms(sine(48000, 8000.0, 48000.0, 1.0f), coefficients);
    CHECK(high < 0.2 * low);
}

TEST_CASE(highpass_attenuates_low_and_passes_high) {
    const auto coefficients = highpassCoefficients(48000.0, 1000.0, 0.70710678);
    const double low = filteredRms(sine(48000, 100.0, 48000.0, 1.0f), coefficients);
    const double high = filteredRms(sine(48000, 8000.0, 48000.0, 1.0f), coefficients);
    CHECK(low < 0.2 * high);
}
