#include "masterizer/domain/dsp/TransientShaper.h"

#include "masterizer/domain/dsp/CrestFactor.h"
#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/SignalsProgram.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::applyTransientShaper;
using masterizer::domain::dsp::crestFactorDb;
using masterizer::domain::dsp::rms;
using masterizer::domain::dsp::TransientShaperSettings;
using masterizer::test::sine;
using masterizer::test::transientRich;

TEST_CASE(zero_settings_leave_the_signal_unchanged) {
    auto data = transientRich(48000, 4000, 0.6, 1.0f);
    const double before = rms(data.data(), 48000);
    applyTransientShaper(data.data(), 48000, 48000.0, TransientShaperSettings{});
    CHECK_NEAR(rms(data.data(), 48000), before, 1e-6);
}

TEST_CASE(attack_boost_increases_crest) {
    auto data = transientRich(48000, 4000, 0.6, 1.0f);
    const double before = crestFactorDb(data.data(), 48000);
    TransientShaperSettings settings;
    settings.attackGainDb = 6.0;
    applyTransientShaper(data.data(), 48000, 48000.0, settings);
    CHECK(crestFactorDb(data.data(), 48000) > before);
}

TEST_CASE(sustain_boost_raises_a_steady_tone) {
    auto data = sine(48000, 1000.0, 48000.0, 0.3f);
    const double before = rms(data.data(), 48000);
    TransientShaperSettings settings;
    settings.sustainGainDb = 6.0;
    applyTransientShaper(data.data(), 48000, 48000.0, settings);
    CHECK(rms(data.data(), 48000) > 1.5 * before);
}
