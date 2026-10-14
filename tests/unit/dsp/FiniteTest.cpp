#include "masterizer/domain/dsp/Finite.h"

#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <limits>

using masterizer::domain::dsp::allFinite;
using masterizer::test::silence;

TEST_CASE(empty_buffer_is_finite) {
    CHECK(allFinite(nullptr, 0));
}

TEST_CASE(silence_is_finite) {
    const auto data = silence(4);
    CHECK(allFinite(data.data(), static_cast<int>(data.size())));
}

TEST_CASE(nan_is_not_finite) {
    const float data[2] = {0.0f, std::numeric_limits<float>::quiet_NaN()};
    CHECK(!allFinite(data, 2));
}

TEST_CASE(infinity_is_not_finite) {
    const float data[2] = {std::numeric_limits<float>::infinity(), 0.0f};
    CHECK(!allFinite(data, 2));
}
