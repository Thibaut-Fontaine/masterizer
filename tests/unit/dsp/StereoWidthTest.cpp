#include "masterizer/domain/dsp/StereoWidth.h"

#include "masterizer/domain/dsp/MidSide.h"
#include "support/Checks.h"
#include "support/Stereo.h"
#include "support/TestRegistry.h"

#include <cstddef>
#include <vector>

using masterizer::domain::dsp::applyStereoWidth;
using masterizer::domain::dsp::sideEnergy;
using masterizer::domain::dsp::widthScaleFor;
using masterizer::test::wideStereo;

TEST_CASE(a_width_of_one_keeps_the_signal) {
    const auto pair = wideStereo(48000, 5u, 0.5f);
    auto left = pair.left;
    auto right = pair.right;
    applyStereoWidth(left.data(), right.data(), 48000, 1.0);
    for (std::size_t i = 0; i < left.size(); ++i) {
        CHECK_NEAR(left[i], pair.left[i], 1e-6);
        CHECK_NEAR(right[i], pair.right[i], 1e-6);
    }
}

TEST_CASE(a_width_of_zero_is_mono) {
    const auto pair = wideStereo(48000, 5u, 0.5f);
    auto left = pair.left;
    auto right = pair.right;
    applyStereoWidth(left.data(), right.data(), 48000, 0.0);
    for (std::size_t i = 0; i < left.size(); ++i) {
        CHECK_NEAR(left[i], right[i], 1e-6);
    }
}

TEST_CASE(a_width_of_two_quadruples_the_side_energy) {
    const auto pair = wideStereo(48000, 5u, 0.5f);
    auto left = pair.left;
    auto right = pair.right;
    const double before = sideEnergy(pair.left.data(), pair.right.data(), 48000);
    applyStereoWidth(left.data(), right.data(), 48000, 2.0);
    CHECK_NEAR(sideEnergy(left.data(), right.data(), 48000) / before, 4.0, 0.05);
}

TEST_CASE(width_scale_targets_a_requested_width) {
    CHECK_NEAR(widthScaleFor(0.5, 0.5), 1.0, 1e-9);
    CHECK_NEAR(widthScaleFor(0.5, 0.0), 0.0, 1e-9);
}
