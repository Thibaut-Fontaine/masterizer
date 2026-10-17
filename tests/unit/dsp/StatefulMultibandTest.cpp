#include "masterizer/domain/dsp/MultibandProcessor.h"

#include "masterizer/domain/dsp/MultibandCompressor.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

using masterizer::domain::dsp::applyMultibandCompressor;
using masterizer::domain::dsp::MultibandCompressorSettings;
using masterizer::domain::dsp::MultibandProcessor;
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

MultibandCompressorSettings settings() {
    MultibandCompressorSettings value;
    value.crossoverHz = 200.0;
    value.low.thresholdDb = -20.0;
    value.low.ratio = 2.0;
    value.low.attackSeconds = 0.01;
    value.low.releaseSeconds = 0.1;
    value.low.kneeDb = 6.0;
    value.high.thresholdDb = -24.0;
    value.high.ratio = 2.0;
    value.high.attackSeconds = 0.01;
    value.high.releaseSeconds = 0.1;
    value.high.kneeDb = 6.0;
    return value;
}

}  // namespace

TEST_CASE(multiband_processor_matches_the_offline_compressor) {
    const int count = 4096;
    const std::vector<float> input = whiteNoise(count, 41u, 0.6f);
    const MultibandCompressorSettings configuration = settings();
    std::vector<float> expected = input;
    applyMultibandCompressor(expected.data(), count, 48000.0, configuration);
    std::vector<float> actual = input;
    MultibandProcessor processor(48000.0, configuration);
    processor.prepare(count);
    processor.process(actual.data(), count);
    CHECK(matches(expected, actual, 1e-9));
}

TEST_CASE(multiband_processor_is_block_size_independent) {
    const int count = 4096;
    const std::vector<float> input = whiteNoise(count, 42u, 0.6f);
    const MultibandCompressorSettings configuration = settings();
    std::vector<float> whole = input;
    MultibandProcessor wholeProcessor(48000.0, configuration);
    wholeProcessor.prepare(count);
    wholeProcessor.process(whole.data(), count);
    std::vector<float> chunked = input;
    MultibandProcessor chunkedProcessor(48000.0, configuration);
    chunkedProcessor.prepare(257);
    for (int position = 0; position < count; position += 257) {
        const int size = std::min(257, count - position);
        chunkedProcessor.process(chunked.data() + position, size);
    }
    CHECK(matches(whole, chunked, 1e-9));
}
