#include "masterizer/domain/dsp/Compressor.h"

#include "masterizer/domain/dsp/Peak.h"
#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::applyCompressor;
using masterizer::domain::dsp::CompressorSettings;
using masterizer::domain::dsp::peakAbs;
using masterizer::domain::dsp::rms;
using masterizer::test::sine;

namespace {

CompressorSettings settings(double thresholdDb, double ratio, double makeupDb = 0.0) {
    CompressorSettings s;
    s.thresholdDb = thresholdDb;
    s.ratio = ratio;
    s.kneeDb = 0.0;
    s.makeupDb = makeupDb;
    return s;
}

}  // namespace

TEST_CASE(signal_below_threshold_is_unchanged) {
    auto data = sine(48000, 1000.0, 48000.0, 0.1f);
    const double before = rms(data.data(), 48000);
    applyCompressor(data.data(), 48000, 48000.0, settings(-6.0, 4.0));
    CHECK_NEAR(rms(data.data(), 48000), before, 1e-6);
}

TEST_CASE(signal_above_threshold_is_reduced) {
    auto data = sine(48000, 1000.0, 48000.0, 0.5f);
    const double before = rms(data.data(), 48000);
    applyCompressor(data.data(), 48000, 48000.0, settings(-18.0, 4.0));
    CHECK(rms(data.data(), 48000) < 0.8 * before);
}

TEST_CASE(a_higher_ratio_reduces_more) {
    auto gentle = sine(48000, 1000.0, 48000.0, 0.5f);
    auto firm = gentle;
    applyCompressor(gentle.data(), 48000, 48000.0, settings(-18.0, 2.0));
    applyCompressor(firm.data(), 48000, 48000.0, settings(-18.0, 8.0));
    CHECK(rms(firm.data(), 48000) < rms(gentle.data(), 48000));
}

TEST_CASE(makeup_gain_raises_a_below_threshold_signal) {
    auto data = sine(48000, 1000.0, 48000.0, 0.1f);
    const double before = rms(data.data(), 48000);
    applyCompressor(data.data(), 48000, 48000.0, settings(-6.0, 4.0, 6.0206));
    CHECK_NEAR(rms(data.data(), 48000) / before, 2.0, 1e-3);
}
