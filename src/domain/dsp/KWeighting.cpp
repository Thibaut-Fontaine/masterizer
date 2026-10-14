#include "masterizer/domain/dsp/KWeighting.h"

#include "masterizer/domain/common/Types.h"

#include <cmath>

namespace masterizer::domain::dsp {
namespace {

// BS.1770-4 / libebur128 analytic design parameters.
constexpr double kShelfFrequency = 1681.974450955533;
constexpr double kShelfGainDb = 3.999843853973347;
constexpr double kShelfQ = 0.7071752369554196;
constexpr double kShelfVbExponent = 0.4996667741545416;
constexpr double kHighPassFrequency = 38.13547087602444;
constexpr double kHighPassQ = 0.5003270373238773;

}  // namespace

BiquadCoefficients kWeightingShelf(double sampleRate) {
    const double k = std::tan(domain::kPi * kShelfFrequency / sampleRate);
    const double vh = std::pow(10.0, kShelfGainDb / 20.0);
    const double vb = std::pow(vh, kShelfVbExponent);
    const double a0 = 1.0 + k / kShelfQ + k * k;
    return {(vh + vb * k / kShelfQ + k * k) / a0,
            2.0 * (k * k - vh) / a0,
            (vh - vb * k / kShelfQ + k * k) / a0,
            2.0 * (k * k - 1.0) / a0,
            (1.0 - k / kShelfQ + k * k) / a0};
}

BiquadCoefficients kWeightingHighPass(double sampleRate) {
    const double k = std::tan(domain::kPi * kHighPassFrequency / sampleRate);
    const double a0 = 1.0 + k / kHighPassQ + k * k;
    return {1.0, -2.0, 1.0,
            2.0 * (k * k - 1.0) / a0,
            (1.0 - k / kHighPassQ + k * k) / a0};
}

}  // namespace masterizer::domain::dsp
