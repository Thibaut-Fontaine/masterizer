#include "masterizer/domain/dsp/ShelfDesign.h"

#include <cmath>

namespace masterizer::domain::dsp {
namespace {

constexpr double kPi = 3.14159265358979323846;

struct ShelfTerms {
    double a;
    double cosW0;
    double twoSqrtAAlpha;
};

ShelfTerms terms(double sampleRate, double frequencyHz, double q, double gainDb) {
    const double a = std::pow(10.0, gainDb / 40.0);
    const double w0 = 2.0 * kPi * frequencyHz / sampleRate;
    const double alpha = std::sin(w0) / (2.0 * q);
    return {a, std::cos(w0), 2.0 * std::sqrt(a) * alpha};
}

}  // namespace

BiquadCoefficients lowShelfCoefficients(double sampleRate, double frequencyHz, double q,
                                        double gainDb) {
    const ShelfTerms t = terms(sampleRate, frequencyHz, q, gainDb);
    const double a0 = (t.a + 1.0) + (t.a - 1.0) * t.cosW0 + t.twoSqrtAAlpha;
    return {t.a * ((t.a + 1.0) - (t.a - 1.0) * t.cosW0 + t.twoSqrtAAlpha) / a0,
            2.0 * t.a * ((t.a - 1.0) - (t.a + 1.0) * t.cosW0) / a0,
            t.a * ((t.a + 1.0) - (t.a - 1.0) * t.cosW0 - t.twoSqrtAAlpha) / a0,
            -2.0 * ((t.a - 1.0) + (t.a + 1.0) * t.cosW0) / a0,
            ((t.a + 1.0) + (t.a - 1.0) * t.cosW0 - t.twoSqrtAAlpha) / a0};
}

BiquadCoefficients highShelfCoefficients(double sampleRate, double frequencyHz, double q,
                                         double gainDb) {
    const ShelfTerms t = terms(sampleRate, frequencyHz, q, gainDb);
    const double a0 = (t.a + 1.0) - (t.a - 1.0) * t.cosW0 + t.twoSqrtAAlpha;
    return {t.a * ((t.a + 1.0) + (t.a - 1.0) * t.cosW0 + t.twoSqrtAAlpha) / a0,
            -2.0 * t.a * ((t.a - 1.0) + (t.a + 1.0) * t.cosW0) / a0,
            t.a * ((t.a + 1.0) + (t.a - 1.0) * t.cosW0 - t.twoSqrtAAlpha) / a0,
            2.0 * ((t.a - 1.0) - (t.a + 1.0) * t.cosW0) / a0,
            ((t.a + 1.0) - (t.a - 1.0) * t.cosW0 - t.twoSqrtAAlpha) / a0};
}

}  // namespace masterizer::domain::dsp
