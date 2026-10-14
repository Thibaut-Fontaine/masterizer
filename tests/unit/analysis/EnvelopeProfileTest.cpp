#include "masterizer/domain/analysis/EnvelopeProfile.h"

#include "support/Checks.h"
#include "support/Signals.h"
#include "support/SignalsProgram.h"
#include "support/TestRegistry.h"

using masterizer::domain::analysis::analyzeEnvelope;
using masterizer::test::silence;
using masterizer::test::sine;
using masterizer::test::transientRich;

TEST_CASE(sustained_sine_has_low_percussive_prominence) {
    const auto data = sine(48000, 1000.0, 48000.0, 1.0f);
    const auto profile = analyzeEnvelope(data.data(), 48000, 48000.0);
    CHECK(profile.percussiveProminence < 0.1);
    CHECK_NEAR(profile.crestDb, 3.0103, 0.01);
}

TEST_CASE(transient_signal_has_high_percussive_prominence) {
    const auto data = transientRich(48000, 4000, 0.6, 1.0f);
    const auto profile = analyzeEnvelope(data.data(), 48000, 48000.0);
    CHECK(profile.percussiveProminence > 0.5);
    CHECK(profile.transientDensity > 0.0);
}

TEST_CASE(silence_has_no_prominence) {
    const auto data = silence(48000);
    const auto profile = analyzeEnvelope(data.data(), 48000, 48000.0);
    CHECK_NEAR(profile.percussiveProminence, 0.0, 1e-12);
    CHECK_NEAR(profile.peak, 0.0, 1e-12);
}
