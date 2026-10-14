#include "masterizer/domain/dsp/Correlation.h"

#include <cmath>

namespace masterizer::domain::dsp {

double correlation(const float* left, const float* right, int count) {
    if (left == nullptr || right == nullptr || count <= 0) {
        return 0.0;
    }
    double sumLeftRight = 0.0;
    double sumLeftSquared = 0.0;
    double sumRightSquared = 0.0;
    for (int i = 0; i < count; ++i) {
        const double l = static_cast<double>(left[i]);
        const double r = static_cast<double>(right[i]);
        sumLeftRight += l * r;
        sumLeftSquared += l * l;
        sumRightSquared += r * r;
    }
    const double denominator = std::sqrt(sumLeftSquared * sumRightSquared);
    return denominator > 0.0 ? sumLeftRight / denominator : 0.0;
}

}  // namespace masterizer::domain::dsp
