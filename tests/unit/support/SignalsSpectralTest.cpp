#include "support/SignalsSpectral.h"

#include "masterizer/domain/dsp/Finite.h"
#include "masterizer/domain/dsp/Peak.h"
#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::domain::dsp::allFinite;
using masterizer::domain::dsp::peakAbs;
using masterizer::domain::dsp::rms;
using masterizer::test::pinkNoise;
using masterizer::test::sweepLog;

namespace {

int zeroCrossings(const std::vector<float>& data, int from, int to) {
    int changes = 0;
    for (int i = from + 1; i < to; ++i) {
        const bool before = data[static_cast<std::size_t>(i - 1)] < 0.0f;
        const bool after = data[static_cast<std::size_t>(i)] < 0.0f;
        if (before != after) {
            ++changes;
        }
    }
    return changes;
}

}  // namespace

TEST_CASE(sweep_stays_within_amplitude_and_finite) {
    const auto data = sweepLog(48000, 20.0, 20000.0, 48000.0, 0.8f);
    CHECK(data.size() == 48000u);
    CHECK(peakAbs(data.data(), 48000) <= 0.8 + 1e-6);
    CHECK(allFinite(data.data(), 48000));
}

TEST_CASE(sweep_rises_in_frequency) {
    const auto data = sweepLog(48000, 100.0, 8000.0, 48000.0, 1.0f);
    const int early = zeroCrossings(data, 0, 4000);
    const int late = zeroCrossings(data, 44000, 48000);
    CHECK(late > early * 4);
}

TEST_CASE(pink_noise_is_deterministic_and_nonzero) {
    const auto first = pinkNoise(48000, 7u, 0.5f);
    const auto second = pinkNoise(48000, 7u, 0.5f);
    CHECK(first == second);
    CHECK(rms(first.data(), 48000) > 0.0);
    CHECK(allFinite(first.data(), 48000));
}
