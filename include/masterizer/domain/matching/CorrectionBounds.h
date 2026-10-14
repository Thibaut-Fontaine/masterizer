#pragma once

namespace masterizer::domain::matching {

/// Internal, configurable limits for proposed corrections.
struct CorrectionBounds {
    double maxEqCutDb = 6.0;
    double maxEqBoostDb = 6.0;
    double minEqDifferenceDb = 0.5;
    double minConfidence = 0.2;
};

}  // namespace masterizer::domain::matching
