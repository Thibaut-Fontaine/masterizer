#include "masterizer/domain/common/OctaveBands.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <cmath>

namespace {

using masterizer::domain::OctaveBand;
using masterizer::domain::bandCount;
using masterizer::domain::centreHz;
using masterizer::domain::lowerEdgeHz;
using masterizer::domain::upperEdgeHz;

}  // namespace

TEST_CASE(band_layout_has_ten_octave_bands) {
    CHECK(bandCount() == 10);
}

TEST_CASE(low_band_edges_match_spec) {
    CHECK_NEAR(lowerEdgeHz(OctaveBand::Low), 45.0, 1e-9);
    CHECK_NEAR(upperEdgeHz(OctaveBand::Low), 90.0, 1e-9);
}

TEST_CASE(low_mid_centre_is_geometric_mean) {
    CHECK_NEAR(lowerEdgeHz(OctaveBand::LowMid), 180.0, 1e-9);
    CHECK_NEAR(upperEdgeHz(OctaveBand::LowMid), 360.0, 1e-9);
    CHECK_NEAR(centreHz(OctaveBand::LowMid), 254.55844122715712, 1e-6);
}

TEST_CASE(bands_are_contiguous) {
    bool ok = true;
    for (int i = 0; i < bandCount() - 1; ++i) {
        const auto lower = static_cast<OctaveBand>(i);
        const auto upper = static_cast<OctaveBand>(i + 1);
        if (upperEdgeHz(lower) != lowerEdgeHz(upper)) {
            ok = false;
        }
    }
    CHECK(ok);
}

TEST_CASE(edge_bands_are_open) {
    CHECK_NEAR(lowerEdgeHz(OctaveBand::Sub), 0.0, 1e-9);
    CHECK(std::isinf(upperEdgeHz(OctaveBand::VeryHigh)));
    CHECK_NEAR(centreHz(OctaveBand::Sub), 0.0, 1e-9);
}
