#include "masterizer/application/OutputValidator.h"
#include "masterizer/application/ProcessingChain.h"
#include "masterizer/domain/analysis/SourceAnalyzer.h"
#include "masterizer/domain/dsp/Finite.h"
#include "masterizer/domain/dsp/Peak.h"
#include "masterizer/domain/dsp/Rms.h"
#include "support/Buffers.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/Stereo.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::application::ChainSettings;
using masterizer::application::process;
using masterizer::application::validateOutput;
using masterizer::domain::analysis::AnalysisSettings;
using masterizer::domain::analysis::analyzeSource;
using masterizer::domain::audio::AudioBuffer;
using masterizer::domain::dsp::allFinite;
using masterizer::domain::dsp::peakAbs;
using masterizer::domain::dsp::rms;
using masterizer::test::monoBuffer;
using masterizer::test::silence;
using masterizer::test::sine;
using masterizer::test::wideStereo;

namespace {

constexpr AnalysisSettings kSettings{1024, 256};

}  // namespace

TEST_CASE(identical_source_and_reference_stay_close) {
    const auto data = sine(96000, 997.0, 48000.0, 0.3f);
    const auto buffer = monoBuffer(data, 48000.0);
    const auto profile = analyzeSource(buffer, kSettings);
    ChainSettings settings;
    settings.monoBassEnabled = false;
    const auto result = process(buffer, profile, profile, settings);
    CHECK_NEAR(rms(result.buffer.channels[0].data(), 96000) / rms(data.data(), 96000), 1.0, 1e-3);
}

TEST_CASE(unreachable_loudness_target_is_reported) {
    const auto data = sine(48000 * 5, 997.0, 48000.0, 0.05f);
    const auto report = validateOutput(monoBuffer(data, 48000.0), -6.0, -1.0);
    CHECK(report.shortfallLu > 0.0);
}

TEST_CASE(full_chain_output_is_finite_and_within_ceiling) {
    const auto pair = wideStereo(96000, 9u, 0.6f);
    AudioBuffer input;
    input.channels = {pair.left, pair.right};
    input.sampleRate = 48000.0;
    const auto profile = analyzeSource(input, kSettings);
    ChainSettings settings;
    settings.saturationEnabled = true;
    settings.saturation = {6.0, 0.5};
    settings.transientEnabled = true;
    settings.transient = {3.0, 0.0};
    settings.glueEnabled = true;
    settings.glue.thresholdDb = -18.0;
    settings.glue.ratio = 2.0;
    settings.multibandEnabled = true;
    settings.dynamicEqEnabled = true;
    settings.stereoEnabled = true;
    settings.stereoWidthScale = 1.2;
    const auto result = process(input, profile, profile, settings);
    CHECK(allFinite(result.buffer.channels[0].data(), 96000));
    CHECK(allFinite(result.buffer.channels[1].data(), 96000));
    CHECK(peakAbs(result.buffer.channels[0].data(), 96000) <= 0.8915);
    CHECK(peakAbs(result.buffer.channels[1].data(), 96000) <= 0.8915);
}

TEST_CASE(silence_through_the_chain_is_safe) {
    const auto data = silence(48000);
    const auto buffer = monoBuffer(data, 48000.0);
    const auto profile = analyzeSource(buffer, kSettings);
    const auto result = process(buffer, profile, profile, ChainSettings{});
    CHECK(allFinite(result.buffer.channels[0].data(), 48000));
    for (float sample : result.buffer.channels[0]) {
        CHECK_NEAR(sample, 0.0, 1e-9);
    }
}
