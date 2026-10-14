#include "masterizer/domain/dsp/MidSide.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::midEnergy;
using masterizer::domain::dsp::midSample;
using masterizer::domain::dsp::sideEnergy;
using masterizer::domain::dsp::sideSample;

TEST_CASE(mid_and_side_of_correlated_stereo) {
    CHECK_NEAR(midSample(1.0, 1.0), 1.0, 1e-12);
    CHECK_NEAR(sideSample(1.0, 1.0), 0.0, 1e-12);
}

TEST_CASE(mid_and_side_of_anti_phase_stereo) {
    CHECK_NEAR(midSample(1.0, -1.0), 0.0, 1e-12);
    CHECK_NEAR(sideSample(1.0, -1.0), 1.0, 1e-12);
}

TEST_CASE(mono_signal_has_no_side_energy) {
    const float data[2] = {0.5f, 0.5f};
    CHECK_NEAR(midEnergy(data, data, 2), 0.25, 1e-12);
    CHECK_NEAR(sideEnergy(data, data, 2), 0.0, 1e-12);
}

TEST_CASE(anti_phase_signal_has_no_mid_energy) {
    const float left[2] = {1.0f, -1.0f};
    const float right[2] = {-1.0f, 1.0f};
    CHECK_NEAR(midEnergy(left, right, 2), 0.0, 1e-12);
    CHECK_NEAR(sideEnergy(left, right, 2), 1.0, 1e-12);
}
