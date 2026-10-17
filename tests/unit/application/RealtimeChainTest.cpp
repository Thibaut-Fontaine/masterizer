#include "masterizer/application/RealtimeChain.h"

#include "masterizer/domain/dsp/Finite.h"
#include "masterizer/domain/dsp/Gain.h"
#include "masterizer/domain/dsp/Peak.h"
#include "support/Checks.h"
#include "support/Stereo.h"
#include "support/TestRegistry.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

using masterizer::application::ChainSettings;
using masterizer::application::RealtimeChain;
using masterizer::application::RealtimeChainBands;
using masterizer::domain::dsp::allFinite;
using masterizer::domain::dsp::dbToLinear;
using masterizer::domain::dsp::EqBand;
using masterizer::domain::dsp::EqShape;
using masterizer::domain::dsp::peakAbs;
using masterizer::test::wideStereo;

namespace {

ChainSettings fullSettings() {
    ChainSettings settings;
    settings.monoBassEnabled = true;
    settings.saturationEnabled = true;
    settings.saturation = {3.0, 0.5};
    settings.eqEnabled = true;
    settings.fineEqEnabled = true;
    settings.transientEnabled = true;
    settings.transient = {2.0, 0.0};
    settings.glueEnabled = true;
    settings.glue.thresholdDb = -20.0;
    settings.glue.ratio = 2.0;
    settings.glue.attackSeconds = 0.01;
    settings.glue.releaseSeconds = 0.1;
    settings.glue.kneeDb = 6.0;
    settings.multibandEnabled = true;
    settings.multiband.crossoverHz = 200.0;
    settings.multiband.low = settings.glue;
    settings.multiband.high = settings.glue;
    settings.dynamicEqEnabled = true;
    settings.dynamicEq = {3000.0, 1.0, -24.0, 4.0, 0.01, 0.1};
    settings.tapeEnabled = true;
    settings.tape = {1.5, 0.5, -0.5, 0.7};
    settings.stereoEnabled = true;
    settings.stereoWidthScale = 1.1;
    settings.limiterEnabled = true;
    settings.limiter.ceilingDb = -1.0;
    settings.limiter.releaseSeconds = 0.1;
    settings.outputGainDb = 3.0;
    return settings;
}

RealtimeChainBands fullBands() {
    RealtimeChainBands bands;
    bands.eq = {{120.0, 1.4, -2.0, EqShape::Peaking},
                {8000.0, 0.7071067811865476, 1.5, EqShape::HighShelf}};
    bands.resonance = {{60.0, 8.0, -4.0, EqShape::Peaking}};
    bands.fineEq = {{3000.0, 2.0, -1.0, EqShape::Peaking}};
    return bands;
}

}  // namespace

TEST_CASE(realtime_chain_is_block_size_independent) {
    const int count = 4096;
    const auto pair = wideStereo(count, 51u, 0.4f);
    const ChainSettings settings = fullSettings();
    const RealtimeChainBands bands = fullBands();
    std::vector<float> wholeLeft = pair.left;
    std::vector<float> wholeRight = pair.right;
    RealtimeChain whole(48000.0, count, settings, bands);
    whole.process(wholeLeft.data(), wholeRight.data(), count);
    std::vector<float> chunkedLeft = pair.left;
    std::vector<float> chunkedRight = pair.right;
    RealtimeChain chunked(48000.0, 257, settings, bands);
    for (int position = 0; position < count; position += 257) {
        const int size = std::min(257, count - position);
        chunked.process(chunkedLeft.data() + position, chunkedRight.data() + position, size);
    }
    for (int i = 0; i < count; ++i) {
        CHECK_NEAR(wholeLeft[static_cast<std::size_t>(i)],
                   chunkedLeft[static_cast<std::size_t>(i)], 1e-5);
        CHECK_NEAR(wholeRight[static_cast<std::size_t>(i)],
                   chunkedRight[static_cast<std::size_t>(i)], 1e-5);
    }
}

TEST_CASE(realtime_chain_reports_the_limiter_latency) {
    const ChainSettings settings = fullSettings();
    const RealtimeChain chain(48000.0, 512, settings, fullBands());
    CHECK(chain.latencySamples() == 240);
}

TEST_CASE(realtime_chain_stays_finite_and_below_the_ceiling) {
    const int count = 8192;
    const auto pair = wideStereo(count, 52u, 1.0f);
    std::vector<float> left = pair.left;
    std::vector<float> right = pair.right;
    const ChainSettings settings = fullSettings();
    RealtimeChain chain(48000.0, 512, settings, fullBands());
    chain.process(left.data(), right.data(), count);
    CHECK(allFinite(left.data(), count));
    CHECK(allFinite(right.data(), count));
    CHECK(peakAbs(left.data(), count) <= dbToLinear(-1.0) + 1e-6);
    CHECK(peakAbs(right.data(), count) <= dbToLinear(-1.0) + 1e-6);
}
