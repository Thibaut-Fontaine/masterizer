#include "masterizer/domain/dsp/CrestFactor.h"

#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

using masterizer::domain::dsp::crestFactorDb;
using masterizer::test::silence;
using masterizer::test::sine;

TEST_CASE(crest_factor_of_full_scale_sine_is_about_three_db) {
    const auto data = sine(48000, 1000.0, 48000.0, 1.0f);
    CHECK_NEAR(crestFactorDb(data.data(), static_cast<int>(data.size())), 3.0103, 0.01);
}

TEST_CASE(crest_factor_of_dc_is_zero_db) {
    const float data[4] = {0.5f, 0.5f, 0.5f, 0.5f};
    CHECK_NEAR(crestFactorDb(data, 4), 0.0, 1e-9);
}

TEST_CASE(crest_factor_of_silence_is_zero) {
    const auto data = silence(16);
    CHECK_NEAR(crestFactorDb(data.data(), static_cast<int>(data.size())), 0.0, 1e-12);
}
