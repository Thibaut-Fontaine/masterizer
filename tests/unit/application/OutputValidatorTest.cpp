#include "masterizer/application/OutputValidator.h"

#include "support/Buffers.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

using masterizer::application::validateOutput;
using masterizer::test::monoBuffer;
using masterizer::test::sine;

TEST_CASE(a_quieter_than_target_render_reports_a_shortfall) {
    const auto data = sine(48000 * 5, 997.0, 48000.0, 0.1f);
    const auto report = validateOutput(monoBuffer(data, 48000.0), -6.0, -1.0);
    CHECK(report.shortfallLu > 0.0);
    CHECK(!report.ceilingExceeded);
}

TEST_CASE(a_render_at_target_has_no_shortfall) {
    const auto data = sine(48000 * 5, 997.0, 48000.0, 1.0f);
    const auto report = validateOutput(monoBuffer(data, 48000.0), -3.0103, -1.0);
    CHECK_NEAR(report.shortfallLu, 0.0, 0.05);
    CHECK(report.ceilingExceeded);  // full-scale sine exceeds a -1 dBTP ceiling
}
