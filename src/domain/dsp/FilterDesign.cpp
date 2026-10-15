#include "masterizer/domain/dsp/FilterDesign.h"

#include <cmath>

namespace masterizer::domain::dsp {
namespace {

constexpr double kTwoPi = 2.0 * 3.14159265358979323846;

struct Omega {
    double w0;
    double cosW0;
    double alpha;
};

Omega omega(double sampleRate, double frequencyHz, double q) {
    const double w0 = kTwoPi * frequencyHz / sampleRate;
    return {w0, std::cos(w0), std::sin(w0) / (2.0 * q)};
}

}  // namespace

BiquadCoefficients lowpassCoefficients(double sampleRate, double frequencyHz, double q) {
    const Omega o = omega(sampleRate, frequencyHz, q);
    const double a0 = 1.0 + o.alpha;
    return {(1.0 - o.cosW0) / 2.0 / a0,
             (1.0 - o.cosW0) / a0,
             (1.0 - o.cosW0) / 2.0 / a0,
             (-2.0 * o.cosW0) / a0,
             (1.0 - o.alpha) / a0};
}

BiquadCoefficients highpassCoefficients(double sampleRate, double frequencyHz, double q) {
    const Omega o = omega(sampleRate, frequencyHz, q);
    const double a0 = 1.0 + o.alpha;
    return {(1.0 + o.cosW0) / 2.0 / a0,
            -(1.0 + o.cosW0) / a0,
            (1.0 + o.cosW0) / 2.0 / a0,
            (-2.0 * o.cosW0) / a0,
            (1.0 - o.alpha) / a0};
}

}  // namespace masterizer::domain::dsp
