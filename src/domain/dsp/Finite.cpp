#include "masterizer/domain/dsp/Finite.h"

#include <cmath>

namespace masterizer::domain::dsp {

bool allFinite(const float* samples, int count) {
    if (samples == nullptr || count <= 0) {
        return true;
    }
    for (int i = 0; i < count; ++i) {
        if (!std::isfinite(samples[i])) {
            return false;
        }
    }
    return true;
}

}  // namespace masterizer::domain::dsp
