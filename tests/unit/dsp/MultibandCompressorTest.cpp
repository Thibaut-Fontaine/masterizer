#include "masterizer/domain/dsp/MultibandCompressor.h"

#include "masterizer/domain/dsp/FilterDesign.h"
#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::domain::dsp::applyMultibandCompressor;
using masterizer::domain::dsp::CompressorSettings;
using masterizer::domain::dsp::MultibandCompressorSettings;
using masterizer::domain::dsp::rms;
using masterizer::test::sine;

namespace {

CompressorSettings neutral() {
    CompressorSettings settings;
    settings.ratio = 1.0;
    settings.thresholdDb = 0.0;
    return settings;
}

CompressorSettings hardLow() {
    CompressorSettings settings;
    settings.ratio = 8.0;
    settings.thresholdDb = -30.0;
    settings.kneeDb = 0.0;
    return settings;
}

}  // namespace

TEST_CASE(neutral_multiband_reconstructs_the_signal) {
    const auto low = sine(48000, 50.0, 48000.0, 0.3f);
    const auto high = sine(48000, 4000.0, 48000.0, 0.3f);
    std::vector<float> data(low.size());
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = low[i] + high[i];
    }
    const double before = rms(data.data(), 48000);
    MultibandCompressorSettings settings;
    settings.low = neutral();
    settings.high = neutral();
    applyMultibandCompressor(data.data(), 48000, 48000.0, settings);
    const double after = rms(data.data(), 48000);
    CHECK(after > 0.9 * before && after < 1.1 * before);
}

TEST_CASE(compressing_the_low_band_reduces_a_low_tone) {
    auto data = sine(48000, 50.0, 48000.0, 0.5f);
    const double before = rms(data.data(), 48000);
    MultibandCompressorSettings settings;
    settings.low = hardLow();
    settings.high = neutral();
    applyMultibandCompressor(data.data(), 48000, 48000.0, settings);
    CHECK(rms(data.data(), 48000) < 0.8 * before);
}

TEST_CASE(compressing_the_high_band_leaves_a_low_tone_alone) {
    auto data = sine(48000, 50.0, 48000.0, 0.5f);
    const double before = rms(data.data(), 48000);
    MultibandCompressorSettings settings;
    settings.low = neutral();
    settings.high = hardLow();
    applyMultibandCompressor(data.data(), 48000, 48000.0, settings);
    CHECK(rms(data.data(), 48000) > 0.9 * before);
}
