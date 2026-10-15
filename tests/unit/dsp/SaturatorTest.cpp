#include "masterizer/domain/dsp/Saturator.h"

#include "masterizer/domain/dsp/Peak.h"
#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::applySaturation;
using masterizer::domain::dsp::peakAbs;
using masterizer::domain::dsp::rms;
using masterizer::domain::dsp::SaturationSettings;
using masterizer::test::sine;

TEST_CASE(zero_drive_leaves_the_signal_unchanged) {
    auto data = sine(48000, 1000.0, 48000.0, 0.5f);
    const double before = rms(data.data(), 48000);
    applySaturation(data.data(), 48000, SaturationSettings{});
    CHECK_NEAR(rms(data.data(), 48000), before, 1e-9);
}

TEST_CASE(drive_compresses_a_full_scale_sine) {
    auto data = sine(48000, 1000.0, 48000.0, 1.0f);
    const double before = peakAbs(data.data(), 48000);
    applySaturation(data.data(), 48000, SaturationSettings{12.0, 1.0});
    CHECK(peakAbs(data.data(), 48000) < before);
}

TEST_CASE(quiet_material_is_level_preserved) {
    auto data = sine(48000, 1000.0, 48000.0, 0.001f);
    const double before = rms(data.data(), 48000);
    applySaturation(data.data(), 48000, SaturationSettings{12.0, 1.0});
    CHECK_NEAR(rms(data.data(), 48000) / before, 1.0, 0.01);
}

TEST_CASE(parallel_mix_of_zero_is_a_no_op) {
    auto data = sine(48000, 1000.0, 48000.0, 0.5f);
    const double before = rms(data.data(), 48000);
    applySaturation(data.data(), 48000, SaturationSettings{12.0, 0.0});
    CHECK_NEAR(rms(data.data(), 48000), before, 1e-9);
}
