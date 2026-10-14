#include "masterizer/domain/matching/MatchIntensity.h"

namespace masterizer::domain::matching {

double scaleEqAmount(double amountDb, double intensity, const CorrectionBounds& bounds) {
    const double scaled = amountDb * intensity;
    if (scaled < -bounds.maxEqCutDb) {
        return -bounds.maxEqCutDb;
    }
    if (scaled > bounds.maxEqBoostDb) {
        return bounds.maxEqBoostDb;
    }
    return scaled;
}

}  // namespace masterizer::domain::matching
