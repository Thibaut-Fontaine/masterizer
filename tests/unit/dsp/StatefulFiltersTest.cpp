#include "masterizer/domain/dsp/EqProcessor.h"
#include "masterizer/domain/dsp/MonoBassProcessor.h"

#include "masterizer/domain/dsp/MonoBass.h"
#include "masterizer/domain/dsp/ParametricEq.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/Stereo.h"
#include "support/TestRegistry.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

using masterizer::domain::dsp::applyEqBands;
using masterizer::domain::dsp::applyMonoBass;
using masterizer::domain::dsp::EqBand;
using masterizer::domain::dsp::EqProcessor;
using masterizer::domain::dsp::EqShape;
using masterizer::domain::dsp::MonoBassProcessor;
using masterizer::domain::dsp::MonoBassSettings;
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

const std::vector<EqBand> kEqBands = {
    {40.0, 0.7071067811865476, 2.0, EqShape::LowShelf},
    {120.0, 1.4, -3.0, EqShape::Peaking},
    {8000.0, 0.7071067811865476, 1.5, EqShape::HighShelf},
};

}  // namespace

TEST_CASE(eq_processor_matches_the_offline_eq) {
    const int count = 4096;
    const std::vector<float> input = whiteNoise(count, 7u, 0.5f);
    std::vector<float> expected = input;
    applyEqBands(expected.data(), count, 48000.0, kEqBands);
    std::vector<float> actual = input;
    EqProcessor processor(48000.0, kEqBands);
    processor.process(actual.data(), count);
    CHECK(matches(expected, actual, 1e-6));
}

TEST_CASE(eq_processor_is_block_size_independent) {
    const int count = 4096;
    const std::vector<float> input = whiteNoise(count, 8u, 0.5f);
    std::vector<float> whole = input;
    EqProcessor wholeProcessor(48000.0, kEqBands);
    wholeProcessor.process(whole.data(), count);
    std::vector<float> chunked = input;
    EqProcessor chunkedProcessor(48000.0, kEqBands);
    for (int position = 0; position < count; position += 257) {
        const int size = std::min(257, count - position);
        chunkedProcessor.process(chunked.data() + position, size);
    }
    CHECK(matches(whole, chunked, 1e-6));
}

TEST_CASE(mono_bass_processor_matches_the_offline_stage) {
    const int count = 4096;
    const auto pair = masterizer::test::wideStereo(count, 11u, 0.4f);
    const MonoBassSettings settings{95.0, true};
    std::vector<float> expectedLeft = pair.left;
    std::vector<float> expectedRight = pair.right;
    applyMonoBass(expectedLeft.data(), expectedRight.data(), count, 48000.0, settings);
    std::vector<float> actualLeft = pair.left;
    std::vector<float> actualRight = pair.right;
    MonoBassProcessor processor(48000.0, settings);
    processor.process(actualLeft.data(), actualRight.data(), count);
    CHECK(matches(expectedLeft, actualLeft, 1e-9));
    CHECK(matches(expectedRight, actualRight, 1e-9));
}

TEST_CASE(mono_bass_processor_is_block_size_independent) {
    const int count = 4096;
    const auto pair = masterizer::test::wideStereo(count, 12u, 0.4f);
    const MonoBassSettings settings{95.0, true};
    std::vector<float> wholeLeft = pair.left;
    std::vector<float> wholeRight = pair.right;
    MonoBassProcessor wholeProcessor(48000.0, settings);
    wholeProcessor.process(wholeLeft.data(), wholeRight.data(), count);
    std::vector<float> chunkedLeft = pair.left;
    std::vector<float> chunkedRight = pair.right;
    MonoBassProcessor chunkedProcessor(48000.0, settings);
    for (int position = 0; position < count; position += 257) {
        const int size = std::min(257, count - position);
        chunkedProcessor.process(chunkedLeft.data() + position, chunkedRight.data() + position,
                                 size);
    }
    CHECK(matches(wholeLeft, chunkedLeft, 1e-9));
    CHECK(matches(wholeRight, chunkedRight, 1e-9));
}
