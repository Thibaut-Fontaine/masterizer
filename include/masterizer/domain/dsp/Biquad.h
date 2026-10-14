#pragma once

namespace masterizer::domain::dsp {

/// Normalised biquad coefficients (a0 is assumed to be 1).
struct BiquadCoefficients {
    double b0;
    double b1;
    double b2;
    double a1;
    double a2;
};

/// Direct Form I state for a single biquad.
struct BiquadState {
    double x1 = 0.0;
    double x2 = 0.0;
    double y1 = 0.0;
    double y2 = 0.0;
};

/// Processes one sample through the biquad, updating `state`.
double processBiquad(const BiquadCoefficients& coefficients, BiquadState& state, double input);

}  // namespace masterizer::domain::dsp
