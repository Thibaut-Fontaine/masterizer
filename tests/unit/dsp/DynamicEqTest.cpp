#include "masterizer/domain/dsp/DynamicEq.h"

#include "masterizer/domain/dsp/FilterDesign.h"
#include "masterizer/domain/dsp/Gain.h"
#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::applyDynamicEqBand;
using masterizer::domain::dsp::DynamicEqBandSettings;
using masterizer::domain::dsp::rms;
using masterizer::test::sine;

namespace {

DynamicEqBandSettings band(double frequencyHz) {
    DynamicEqBandSettings settings;
    settings.frequencyHz = frequencyHz;
    settings.q = 1.0;
    return settings;
}

}  // namespace

TEST_CASE(a_quiet_band_is_left_untouched) {
    auto data = sine(48000, 1000.0, 48000.0, 0.01f);
    const double before = rms(data.data(), 48000);
    applyDynamicEqBand(data.data(), 48000, 48000.0, band(1000.0));
    CHECK_NEAR(rms(data.data(), 48000) / before, 1.0, 0.05);
}

TEST_CASE(a_loud_band_is_attenuated) {
    auto data = sine(48000, 1000.0, 48000.0, 0.5f);
    const double before = rms(data.data(), 48000);
    applyDynamicEqBand(data.data(), 48000, 48000.0, band(1000.0));
    CHECK(rms(data.data(), 48000) < 0.7 * before);
}

TEST_CASE(a_tone_outside_the_band_is_untouched) {
    auto data = sine(48000, 1000.0, 48000.0, 0.5f);
    const double before = rms(data.data(), 48000);
    applyDynamicEqBand(data.data(), 48000, 48000.0, band(6000.0));
    CHECK_NEAR(rms(data.data(), 48000) / before, 1.0, 0.05);
}
