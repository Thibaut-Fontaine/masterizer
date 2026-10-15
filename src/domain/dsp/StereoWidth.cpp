#include "masterizer/domain/dsp/StereoWidth.h"

#include <algorithm>
#include <cmath>

namespace masterizer::domain::dsp {

void applyStereoWidth(float* left, float* right, int count, double widthScale) {
    if (left == nullptr || right == nullptr || count <= 0) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        const double mid = 0.5 * (static_cast<double>(left[i]) + static_cast<double>(right[i]));
        const double side =
            0.5 * (static_cast<double>(left[i]) - static_cast<double>(right[i])) * widthScale;
        left[i] = static_cast<float>(mid + side);
        right[i] = static_cast<float>(mid - side);
    }
}

double widthScaleFor(double currentWidth, double targetWidth) {
    if (currentWidth <= 0.0 || currentWidth >= 1.0) {
        return targetWidth <= currentWidth ? 1.0 : 2.0;
    }
    const double clampedTarget = std::clamp(targetWidth, 0.0, 0.999);
    const double ratioSquared =
        (clampedTarget / (1.0 - clampedTarget)) * ((1.0 - currentWidth) / currentWidth);
    return std::clamp(std::sqrt(ratioSquared), 0.0, 2.0);
}

}  // namespace masterizer::domain::dsp
