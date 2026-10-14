#include "masterizer/domain/analysis/SpectralFraming.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::analysis::hannWindow;
using masterizer::domain::analysis::isPowerOfTwo;

TEST_CASE(power_of_two_detection) {
    CHECK(isPowerOfTwo(1));
    CHECK(isPowerOfTwo(2));
    CHECK(isPowerOfTwo(1024));
    CHECK(!isPowerOfTwo(0));
    CHECK(!isPowerOfTwo(3));
    CHECK(!isPowerOfTwo(-4));
}

TEST_CASE(hann_window_has_zero_ends_and_unit_centre) {
    const auto window = hannWindow(5);
    CHECK(window.size() == 5u);
    CHECK_NEAR(window[0], 0.0, 1e-12);
    CHECK_NEAR(window[4], 0.0, 1e-12);
    CHECK_NEAR(window[2], 1.0, 1e-12);
}
