#include "masterizer/domain/dsp/CompressorProcessor.h"
#include "masterizer/domain/dsp/TransientProcessor.h"

#include "masterizer/domain/dsp/Compressor.h"
#include "masterizer/domain/dsp/TransientShaper.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

using masterizer::domain::dsp::applyCompressor;
using masterizer::domain::dsp::applyTransientShaper;
using masterizer::domain::dsp::CompressorProcessor;
using masterizer::domain::dsp::CompressorSettings;
using masterizer::domain::dsp::TransientProcessor;
using masterizer::domain::dsp::TransientShaperSettings;
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

TEST_CASE(compressor_processor_matches_the_offline_compressor) {
    const int count = 4096;
    const std::vector<float> input = whiteNoise(count, 21u, 0.6f);
    CompressorSettings settings;
    settings.thresholdDb = -20.0;
    settings.ratio = 2.5;
    settings.attackSeconds = 0.01;
    settings.releaseSeconds = 0.1;
    settings.kneeDb = 6.0;
    settings.makeupDb = 1.0;
    std::vector<float> expected = input;
    applyCompressor(expected.data(), count, 48000.0, settings);
    std::vector<float> actual = input;
    CompressorProcessor processor(48000.0, settings);
    processor.process(actual.data(), count);
    CHECK(matches(expected, actual, 1e-9));
}

TEST_CASE(compressor_processor_is_block_size_independent) {
    const int count = 4096;
    const std::vector<float> input = whiteNoise(count, 22u, 0.6f);
    CompressorSettings settings;
    settings.thresholdDb = -20.0;
    settings.ratio = 2.5;
    std::vector<float> whole = input;
    CompressorProcessor wholeProcessor(48000.0, settings);
    wholeProcessor.process(whole.data(), count);
    std::vector<float> chunked = input;
    CompressorProcessor chunkedProcessor(48000.0, settings);
    for (int position = 0; position < count; position += 257) {
        const int size = std::min(257, count - position);
        chunkedProcessor.process(chunked.data() + position, size);
    }
    CHECK(matches(whole, chunked, 1e-9));
}

TEST_CASE(transient_processor_matches_the_offline_shaper) {
    const int count = 4096;
    const std::vector<float> input = whiteNoise(count, 23u, 0.5f);
    const TransientShaperSettings settings{3.0, -1.5, 0.002, 0.05};
    std::vector<float> expected = input;
    applyTransientShaper(expected.data(), count, 48000.0, settings);
    std::vector<float> actual = input;
    TransientProcessor processor(48000.0, settings);
    processor.process(actual.data(), count);
    CHECK(matches(expected, actual, 1e-9));
}

TEST_CASE(transient_processor_is_block_size_independent) {
    const int count = 4096;
    const std::vector<float> input = whiteNoise(count, 24u, 0.5f);
    const TransientShaperSettings settings{3.0, -1.5, 0.002, 0.05};
    std::vector<float> whole = input;
    TransientProcessor wholeProcessor(48000.0, settings);
    wholeProcessor.process(whole.data(), count);
    std::vector<float> chunked = input;
    TransientProcessor chunkedProcessor(48000.0, settings);
    for (int position = 0; position < count; position += 257) {
        const int size = std::min(257, count - position);
        chunkedProcessor.process(chunked.data() + position, size);
    }
    CHECK(matches(whole, chunked, 1e-9));
}
