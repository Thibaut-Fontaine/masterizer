#pragma once

#include "masterizer/domain/dsp/ParametricEq.h"
#include "masterizer/domain/matching/CorrectionBounds.h"

#include <vector>

namespace masterizer::domain::matching {

struct FineEqSettings {
    int maxFilters = 6;
    double minDifferenceDb = 1.0;
    double q = 2.0;
    double minHz = 100.0;
    double maxHz = 10000.0;
};

/// Third-octave EQ design from the fine magnitude spectra. Both spectra are
/// normalised to their own total energy, so the design is level-independent.
/// Returns up to `maxFilters` bounded peaking bands at the largest deviations.
std::vector<dsp::EqBand> designFineEq(const std::vector<double>& sourceSpectrum,
                                      const std::vector<double>& referenceSpectrum,
                                      double sampleRate, int fftSize,
                                      const CorrectionBounds& bounds,
                                      const FineEqSettings& settings = {});

}  // namespace masterizer::domain::matching
