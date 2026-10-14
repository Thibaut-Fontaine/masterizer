#include "support/SignalsProgram.h"

#include "masterizer/domain/dsp/CrestFactor.h"
#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::crestFactorDb;
using masterizer::domain::dsp::rms;
using masterizer::test::bassHeavy;
using masterizer::test::transientRich;

TEST_CASE(transient_signal_has_high_crest_factor) {
    const auto data = transientRich(48000, 4000, 0.6, 1.0f);
    CHECK(data.size() == 48000u);
    CHECK(crestFactorDb(data.data(), 48000) > 6.0);
    CHECK(rms(data.data(), 48000) > 0.0);
}

TEST_CASE(bass_heavy_signal_has_low_crest_factor) {
    const auto data = bassHeavy(48000, 48000.0, 1.0f);
    CHECK(data.size() == 48000u);
    CHECK(crestFactorDb(data.data(), 48000) < 5.0);
    CHECK(rms(data.data(), 48000) > 0.1);
}

TEST_CASE(program_fixtures_are_deterministic) {
    CHECK(transientRich(1000, 100, 0.5, 1.0f) == transientRich(1000, 100, 0.5, 1.0f));
    CHECK(bassHeavy(1000, 48000.0, 1.0f) == bassHeavy(1000, 48000.0, 1.0f));
}
