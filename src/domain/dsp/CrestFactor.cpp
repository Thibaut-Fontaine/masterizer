#include "masterizer/domain/dsp/CrestFactor.h"

#include "masterizer/domain/dsp/Peak.h"
#include "masterizer/domain/dsp/Rms.h"

#include <cmath>

namespace masterizer::domain::dsp {

double crestFactorDb(const float* samples, int count) {
    const double level = rms(samples, count);
    if (level <= 0.0) {
        return 0.0;
    }
    return 20.0 * std::log10(peakAbs(samples, count) / level);
}

}  // namespace masterizer::domain::dsp
