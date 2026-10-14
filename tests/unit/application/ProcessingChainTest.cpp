#include "masterizer/application/ProcessingChain.h"

#include "masterizer/domain/dsp/Gain.h"
#include "masterizer/domain/dsp/Peak.h"
#include "masterizer/domain/dsp/Rms.h"
#include "support/Buffers.h"
#include "support/Checks.h"
#include "support/Profiles.h"
#include "support/Signals.h"
#include "support/Stereo.h"
#include "support/TestRegistry.h"

#include <cstddef>
#include <vector>

using masterizer::application::ChainSettings;
using masterizer::application::process;
using masterizer::domain::audio::AudioBuffer;
using masterizer::domain::dsp::dbToLinear;
using masterizer::domain::dsp::peakAbs;
using masterizer::domain::dsp::rms;
using masterizer::test::antiPhase;
using masterizer::test::monoBuffer;
using masterizer::test::profileWithBands;
using masterizer::test::sine;

namespace {

ChainSettings toneOnly() {
    ChainSettings settings;
    settings.monoBassEnabled = false;
    settings.limiterEnabled = false;
    return settings;
}

}  // namespace

TEST_CASE(zero_intensity_leaves_the_signal_unchanged) {
    const auto data = sine(48000, 1000.0, 48000.0, 0.5f);
    const auto input = monoBuffer(data, 48000.0);
    auto settings = toneOnly();
    settings.matchIntensity = 0.0;
    const auto result = process(input, profileWithBands(std::vector<double>(10, 0.1)),
                                profileWithBands(std::vector<double>(10, 0.1)), settings);
    for (std::size_t i = 0; i < data.size(); ++i) {
        CHECK_NEAR(result.buffer.channels[0][i], data[i], 1e-6);
    }
}

TEST_CASE(full_intensity_cuts_an_over_bright_band) {
    std::vector<double> reference(10, 0.1);
    std::vector<double> source = reference;
    source[5] = 1.0;  // UpperMid far above the reference
    const auto data = sine(48000, 1000.0, 48000.0, 0.5f);
    const auto result = process(monoBuffer(data, 48000.0), profileWithBands(source),
                                profileWithBands(reference), toneOnly());
    CHECK(rms(result.buffer.channels[0].data(), 48000) < 0.9 * rms(data.data(), 48000));
}

TEST_CASE(the_limiter_keeps_the_peak_below_the_ceiling) {
    const auto data = sine(48000, 1000.0, 48000.0, 1.0f);
    ChainSettings settings;
    settings.eqEnabled = false;
    settings.monoBassEnabled = false;
    const auto result = process(monoBuffer(data, 48000.0),
                                profileWithBands(std::vector<double>(10, 0.1)),
                                profileWithBands(std::vector<double>(10, 0.1)), settings);
    CHECK(peakAbs(result.buffer.channels[0].data(), 48000) <= dbToLinear(-1.0) + 1e-3);
}

TEST_CASE(mono_bass_collapses_the_low_side) {
    const auto pair = antiPhase(48000, 50.0, 48000.0, 0.5f);
    AudioBuffer input;
    input.channels = {pair.left, pair.right};
    input.sampleRate = 48000.0;
    ChainSettings settings;
    settings.eqEnabled = false;
    settings.limiterEnabled = false;
    const auto result = process(input, profileWithBands(std::vector<double>(10, 0.1)),
                                profileWithBands(std::vector<double>(10, 0.1)), settings);
    CHECK(rms(result.buffer.channels[0].data(), 48000) < 0.4 * rms(pair.left.data(), 48000));
}
