#include "masterizer/domain/dsp/Gain.h"

#include <cmath>
#include <limits>

namespace masterizer::domain::dsp {

double dbToLinear(double decibels) {
    return std::pow(10.0, decibels / 20.0);
}

double linearToDb(double linearGain) {
    if (!(linearGain > 0.0)) {
        return -std::numeric_limits<double>::infinity();
    }
    return 20.0 * std::log10(linearGain);
}

double clampGainDb(double gainDb, const GainBounds& bounds) {
    if (gainDb < bounds.minDb) {
        return bounds.minDb;
    }
    if (gainDb > bounds.maxDb) {
        return bounds.maxDb;
    }
    return gainDb;
}

bool withinGainBounds(double gainDb, const GainBounds& bounds) {
    return std::isfinite(gainDb) && gainDb >= bounds.minDb && gainDb <= bounds.maxDb;
}

}  // namespace masterizer::domain::dsp
