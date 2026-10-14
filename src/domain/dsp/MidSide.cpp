#include "masterizer/domain/dsp/MidSide.h"

namespace masterizer::domain::dsp {
namespace {

/// Mean square of (left + sign * right) / 2.
double combinationEnergy(const float* left, const float* right, int count, double sign) {
    if (left == nullptr || right == nullptr || count <= 0) {
        return 0.0;
    }
    double sum = 0.0;
    for (int i = 0; i < count; ++i) {
        const double leftValue = static_cast<double>(left[i]);
        const double rightValue = static_cast<double>(right[i]);
        const double value = (leftValue + sign * rightValue) * 0.5;
        sum += value * value;
    }
    return sum / static_cast<double>(count);
}

}  // namespace

double midSample(double left, double right) {
    return (left + right) * 0.5;
}

double sideSample(double left, double right) {
    return (left - right) * 0.5;
}

double midEnergy(const float* left, const float* right, int count) {
    return combinationEnergy(left, right, count, 1.0);
}

double sideEnergy(const float* left, const float* right, int count) {
    return combinationEnergy(left, right, count, -1.0);
}

}  // namespace masterizer::domain::dsp
