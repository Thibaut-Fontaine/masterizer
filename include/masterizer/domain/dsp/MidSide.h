#pragma once

namespace masterizer::domain::dsp {

/// Mid sample, (left + right) / 2.
double midSample(double left, double right);

/// Side sample, (left - right) / 2.
double sideSample(double left, double right);

/// Mean-square energy of the Mid signal across two channels.
double midEnergy(const float* left, const float* right, int count);

/// Mean-square energy of the Side signal across two channels.
double sideEnergy(const float* left, const float* right, int count);

}  // namespace masterizer::domain::dsp
