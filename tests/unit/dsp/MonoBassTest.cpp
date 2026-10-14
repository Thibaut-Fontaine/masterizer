#include "masterizer/domain/dsp/MonoBass.h"

#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::domain::dsp::applyMonoBass;
using masterizer::domain::dsp::MonoBassSettings;
using masterizer::domain::dsp::rms;
using masterizer::test::sine;

namespace {

const MonoBassSettings kDefault{95.0, false};

double stereoRms(const std::vector<float>& left, const std::vector<float>& right) {
    return 0.5 * (rms(left.data(), static_cast<int>(left.size())) +
                  rms(right.data(), static_cast<int>(right.size())));
}

}  // namespace

TEST_CASE(mono_input_is_left_unchanged) {
    auto left = sine(48000, 100.0, 48000.0, 0.5f);
    std::vector<float> right = left;
    applyMonoBass(left.data(), right.data(), 48000, 48000.0, kDefault);
    for (int i = 0; i < 48000; ++i) {
        CHECK_NEAR(left[static_cast<std::size_t>(i)], right[static_cast<std::size_t>(i)], 1e-6);
    }
}

TEST_CASE(low_frequency_side_is_largely_removed) {
    auto left = sine(48000, 50.0, 48000.0, 0.5f);
    std::vector<float> right = left;
    for (float& sample : right) {
        sample = -sample;
    }
    const double before = stereoRms(left, right);
    applyMonoBass(left.data(), right.data(), 48000, 48000.0, kDefault);
    CHECK(stereoRms(left, right) < 0.4 * before);
}

TEST_CASE(high_frequency_side_is_preserved) {
    auto left = sine(48000, 4000.0, 48000.0, 0.5f);
    std::vector<float> right = left;
    for (float& sample : right) {
        sample = -sample;
    }
    const double before = stereoRms(left, right);
    applyMonoBass(left.data(), right.data(), 48000, 48000.0, kDefault);
    CHECK(stereoRms(left, right) > 0.9 * before);
}

TEST_CASE(strict_mono_removes_more_low_side_than_default) {
    auto looseL = sine(48000, 70.0, 48000.0, 0.5f);
    auto strictL = looseL;
    std::vector<float> looseR = looseL;
    std::vector<float> strictR = strictL;
    for (float& sample : looseR) {
        sample = -sample;
    }
    for (float& sample : strictR) {
        sample = -sample;
    }
    applyMonoBass(looseL.data(), looseR.data(), 48000, 48000.0, MonoBassSettings{95.0, false});
    applyMonoBass(strictL.data(), strictR.data(), 48000, 48000.0, MonoBassSettings{95.0, true});
    CHECK(stereoRms(strictL, strictR) < stereoRms(looseL, looseR));
}
