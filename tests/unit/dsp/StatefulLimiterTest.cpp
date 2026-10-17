#include "masterizer/domain/dsp/LookaheadProcessor.h"

#include "masterizer/domain/dsp/Gain.h"
#include "masterizer/domain/dsp/Peak.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

using masterizer::domain::dsp::dbToLinear;
using masterizer::domain::dsp::LookaheadLimiterSettings;
using masterizer::domain::dsp::LookaheadProcessor;
using masterizer::domain::dsp::peakAbs;
using masterizer::test::impulse;
using masterizer::test::sine;

TEST_CASE(limiter_reports_the_lookahead_latency) {
    LookaheadLimiterSettings settings;
    settings.lookaheadSeconds = 0.005;
    settings.ceilingDb = -1.0;
    const LookaheadProcessor processor(48000.0, settings);
    CHECK(processor.latencySamples() == 240);
}

TEST_CASE(limiter_delays_the_signal_by_its_latency) {
    LookaheadLimiterSettings settings;
    settings.lookaheadSeconds = 0.005;
    settings.ceilingDb = -1.0;
    std::vector<float> samples = impulse(1024, 100, 0.8f);
    LookaheadProcessor processor(48000.0, settings);
    processor.process(samples.data(), static_cast<int>(samples.size()));
    int peakIndex = 0;
    float peakValue = 0.0f;
    for (int i = 0; i < static_cast<int>(samples.size()); ++i) {
        if (std::fabs(samples[static_cast<std::size_t>(i)]) > peakValue) {
            peakValue = std::fabs(samples[static_cast<std::size_t>(i)]);
            peakIndex = i;
        }
    }
    CHECK(peakIndex == 100 + processor.latencySamples());
}

TEST_CASE(limiter_keeps_the_output_below_the_ceiling) {
    LookaheadLimiterSettings settings;
    settings.lookaheadSeconds = 0.005;
    settings.ceilingDb = -1.0;
    std::vector<float> samples = sine(4096, 997.0, 48000.0, 1.0f);
    LookaheadProcessor processor(48000.0, settings);
    processor.process(samples.data(), static_cast<int>(samples.size()));
    CHECK(peakAbs(samples.data(), static_cast<int>(samples.size())) <=
          dbToLinear(-1.0) + 1e-6);
}

TEST_CASE(limiter_is_block_size_independent) {
    LookaheadLimiterSettings settings;
    settings.lookaheadSeconds = 0.005;
    settings.ceilingDb = -1.0;
    const std::vector<float> input = sine(4096, 997.0, 48000.0, 1.0f);
    std::vector<float> whole = input;
    LookaheadProcessor wholeProcessor(48000.0, settings);
    wholeProcessor.process(whole.data(), static_cast<int>(whole.size()));
    std::vector<float> chunked = input;
    LookaheadProcessor chunkedProcessor(48000.0, settings);
    for (int position = 0; position < static_cast<int>(chunked.size()); position += 257) {
        const int size = std::min(257, static_cast<int>(chunked.size()) - position);
        chunkedProcessor.process(chunked.data() + position, size);
    }
    CHECK(peakAbs(whole.data(), static_cast<int>(whole.size())) <= dbToLinear(-1.0) + 1e-6);
    for (std::size_t i = 0; i < whole.size(); ++i) {
        CHECK_NEAR(whole[i], chunked[i], 1e-9);
    }
}
