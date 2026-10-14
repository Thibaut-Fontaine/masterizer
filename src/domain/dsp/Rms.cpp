#include "masterizer/domain/dsp/Rms.h"

#include <cmath>

namespace masterizer::domain::dsp {

double meanSquare(const float* samples, int count) {
    if (samples == nullptr || count <= 0) {
        return 0.0;
    }
    double sum = 0.0;
    for (int i = 0; i < count; ++i) {
        const double value = static_cast<double>(samples[i]);
        sum += value * value;
    }
    return sum / static_cast<double>(count);
}

double rms(const float* samples, int count) {
    return std::sqrt(meanSquare(samples, count));
}

}  // namespace masterizer::domain::dsp
