#include "masterizer/domain/dsp/DynamicEqProcessor.h"
#include "masterizer/domain/dsp/TapeProcessor.h"

#include "masterizer/domain/dsp/DynamicEq.h"
#include "masterizer/domain/dsp/TapeStage.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

using masterizer::domain::dsp::applyDynamicEqBand;
using masterizer::domain::dsp::applyTapeStage;
using masterizer::domain::dsp::DynamicEqBandSettings;
using masterizer::domain::dsp::DynamicEqProcessor;
using masterizer::domain::dsp::TapeProcessor;
using masterizer::domain::dsp::TapeSettings;
using masterizer::test::whiteNoise;

namespace {

bool matches(const std::vector<float>& a, const std::vector<float>& b, double tolerance) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::fabs(static_cast<double>(a[i]) - static_cast<double>(b[i])) > tolerance) {
            return false;
        }
    }
    return true;
}

}  // namespace

TEST_CASE(dynamic_eq_processor_matches_the_offline_band) {
    const int count = 4096;
    const std::vector<float> input = whiteNoise(count, 31u, 0.6f);
    const DynamicEqBandSettings settings{3000.0, 1.0, -24.0, 4.0, 0.01, 0.1};
    std::vector<float> expected = input;
    applyDynamicEqBand(expected.data(), count, 48000.0, settings);
    std::vector<float> actual = input;
    DynamicEqProcessor processor(48000.0, settings);
    processor.process(actual.data(), count);
    CHECK(matches(expected, actual, 1e-9));
}

TEST_CASE(dynamic_eq_processor_is_block_size_independent) {
    const int count = 4096;
    const std::vector<float> input = whiteNoise(count, 32u, 0.6f);
    const DynamicEqBandSettings settings{3000.0, 1.0, -24.0, 4.0, 0.01, 0.1};
    std::vector<float> whole = input;
    DynamicEqProcessor wholeProcessor(48000.0, settings);
    wholeProcessor.process(whole.data(), count);
    std::vector<float> chunked = input;
    DynamicEqProcessor chunkedProcessor(48000.0, settings);
    for (int position = 0; position < count; position += 257) {
        const int size = std::min(257, count - position);
        chunkedProcessor.process(chunked.data() + position, size);
    }
    CHECK(matches(whole, chunked, 1e-9));
}

TEST_CASE(tape_processor_matches_the_offline_stage) {
    const int count = 4096;
    const std::vector<float> input = whiteNoise(count, 33u, 0.5f);
    const TapeSettings settings{1.5, 0.5, -0.5, 0.7};
    std::vector<float> expected = input;
    applyTapeStage(expected.data(), count, 48000.0, settings);
    std::vector<float> actual = input;
    TapeProcessor processor(48000.0, settings);
    processor.process(actual.data(), count);
    CHECK(matches(expected, actual, 1e-9));
}

TEST_CASE(tape_processor_is_block_size_independent) {
    const int count = 4096;
    const std::vector<float> input = whiteNoise(count, 34u, 0.5f);
    const TapeSettings settings{1.5, 0.5, -0.5, 0.7};
    std::vector<float> whole = input;
    TapeProcessor wholeProcessor(48000.0, settings);
    wholeProcessor.process(whole.data(), count);
    std::vector<float> chunked = input;
    TapeProcessor chunkedProcessor(48000.0, settings);
    for (int position = 0; position < count; position += 257) {
        const int size = std::min(257, count - position);
        chunkedProcessor.process(chunked.data() + position, size);
    }
    CHECK(matches(whole, chunked, 1e-9));
}
