#include "masterizer/domain/dsp/TapeStage.h"

#include "masterizer/domain/dsp/Peak.h"
#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::applyTapeStage;
using masterizer::domain::dsp::peakAbs;
using masterizer::domain::dsp::rms;
using masterizer::domain::dsp::TapeSettings;
using masterizer::test::sine;

TEST_CASE(a_neutral_tape_stage_is_transparent) {
    auto data = sine(48000, 1000.0, 48000.0, 0.5f);
    const double before = rms(data.data(), 48000);
    applyTapeStage(data.data(), 48000, 48000.0, TapeSettings{0.0, 0.0, 0.0, 1.0});
    CHECK_NEAR(rms(data.data(), 48000), before, 1e-6);
}

TEST_CASE(the_head_bump_lifts_the_low_end) {
    auto data = sine(48000, 60.0, 48000.0, 0.3f);
    const double before = rms(data.data(), 48000);
    applyTapeStage(data.data(), 48000, 48000.0, TapeSettings{0.0, 2.0, 0.0, 1.0});
    const double ratio = rms(data.data(), 48000) / before;
    CHECK(ratio > 1.1 && ratio < 1.5);
}

TEST_CASE(the_hf_rolloff_tames_the_top) {
    auto data = sine(48000, 16000.0, 48000.0, 0.3f);
    const double before = rms(data.data(), 48000);
    applyTapeStage(data.data(), 48000, 48000.0, TapeSettings{0.0, 0.0, -2.0, 1.0});
    const double ratio = rms(data.data(), 48000) / before;
    CHECK(ratio < 0.9 && ratio > 0.5);
}

TEST_CASE(drive_compresses_peaks_and_dry_mix_is_a_no_op) {
    auto driven = sine(48000, 1000.0, 48000.0, 1.0f);
    const double before = peakAbs(driven.data(), 48000);
    applyTapeStage(driven.data(), 48000, 48000.0, TapeSettings{12.0, 0.0, 0.0, 1.0});
    CHECK(peakAbs(driven.data(), 48000) < before);

    auto dry = sine(48000, 1000.0, 48000.0, 0.5f);
    const double dryBefore = rms(dry.data(), 48000);
    applyTapeStage(dry.data(), 48000, 48000.0, TapeSettings{12.0, 2.0, -2.0, 0.0});
    CHECK_NEAR(rms(dry.data(), 48000), dryBefore, 1e-9);
}
