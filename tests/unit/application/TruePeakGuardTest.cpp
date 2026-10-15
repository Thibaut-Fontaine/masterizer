#include "masterizer/application/TruePeakGuard.h"

#include "support/Buffers.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include "masterizer/domain/dsp/Gain.h"
#include "masterizer/domain/dsp/TruePeak.h"

#include <vector>

using masterizer::application::enforceTruePeak;
using masterizer::domain::audio::AudioBuffer;
using masterizer::domain::dsp::linearToDb;
using masterizer::domain::dsp::truePeak;
using masterizer::test::monoBuffer;
using masterizer::test::sine;

TEST_CASE(a_render_below_the_ceiling_is_not_touched) {
    const auto data = sine(48000, 997.0, 48000.0, 0.5f);
    auto buffer = monoBuffer(data, 48000.0);
    const double gainDb = enforceTruePeak(buffer, -1.0);
    CHECK_NEAR(gainDb, 0.0, 1e-12);
    CHECK_NEAR(buffer.channels[0][0], data[0], 1e-9);
}

TEST_CASE(a_render_over_the_ceiling_is_scaled_down) {
    const auto data = sine(48000, 997.0, 48000.0, 1.0f);
    auto buffer = monoBuffer(data, 48000.0);
    const double gainDb = enforceTruePeak(buffer, -1.0);
    CHECK(gainDb < 0.0);
    CHECK(linearToDb(truePeak(buffer.channels[0].data(), 48000)) <= -1.0 + 1e-6);
}
