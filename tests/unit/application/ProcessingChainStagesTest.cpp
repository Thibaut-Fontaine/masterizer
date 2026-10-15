#include "masterizer/application/ProcessingChain.h"

#include "masterizer/domain/dsp/MidSide.h"
#include "masterizer/domain/dsp/Rms.h"
#include "support/Buffers.h"
#include "support/Checks.h"
#include "support/Profiles.h"
#include "support/Signals.h"
#include "support/Stereo.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::application::ChainSettings;
using masterizer::application::process;
using masterizer::domain::audio::AudioBuffer;
using masterizer::domain::dsp::rms;
using masterizer::domain::dsp::sideEnergy;
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

TEST_CASE(saturation_alters_the_signal_when_enabled) {
    const auto data = sine(48000, 1000.0, 48000.0, 0.8f);
    auto settings = toneOnly();
    settings.saturationEnabled = true;
    settings.saturation = {12.0, 1.0};
    const auto result = process(monoBuffer(data, 48000.0),
                                profileWithBands(std::vector<double>(10, 0.1)),
                                profileWithBands(std::vector<double>(10, 0.1)), settings);
    CHECK(rms(result.buffer.channels[0].data(), 48000) < 0.99 * rms(data.data(), 48000));
}

TEST_CASE(stereo_stage_reduces_side_energy) {
    const auto pair = antiPhase(48000, 1000.0, 48000.0, 0.5f);
    AudioBuffer input;
    input.channels = {pair.left, pair.right};
    input.sampleRate = 48000.0;
    auto settings = toneOnly();
    settings.stereoEnabled = true;
    settings.stereoWidthScale = 0.0;
    const auto result = process(input, profileWithBands(std::vector<double>(10, 0.1)),
                                profileWithBands(std::vector<double>(10, 0.1)), settings);
    CHECK(sideEnergy(result.buffer.channels[0].data(), result.buffer.channels[1].data(), 48000) <
          1e-9);
}

TEST_CASE(output_gain_scales_the_render) {
    const auto data = sine(48000, 1000.0, 48000.0, 0.25f);
    ChainSettings settings;
    settings.eqEnabled = false;
    settings.monoBassEnabled = false;
    settings.outputGainDb = 6.0206;
    const auto result = process(monoBuffer(data, 48000.0),
                                profileWithBands(std::vector<double>(10, 0.1)),
                                profileWithBands(std::vector<double>(10, 0.1)), settings);
    CHECK_NEAR(rms(result.buffer.channels[0].data(), 48000) / rms(data.data(), 48000), 2.0, 1e-2);
}
