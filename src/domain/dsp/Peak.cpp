#include "masterizer/domain/dsp/Peak.h"

#include <cmath>

namespace masterizer::domain::dsp {

double peakAbs(const float* samples, int count) {
    if (samples == nullptr || count <= 0) {
        return 0.0;
    }
    double peak = 0.0;
    for (int i = 0; i < count; ++i) {
        const double value = std::fabs(static_cast<double>(samples[i]));
        if (value > peak) {
            peak = value;
        }
    }
    return peak;
}

}  // namespace masterizer::domain::dsp
