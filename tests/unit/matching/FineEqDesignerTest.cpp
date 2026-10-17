#include "masterizer/domain/matching/FineEqDesigner.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <cmath>
#include <cstddef>
#include <vector>

using masterizer::domain::matching::CorrectionBounds;
using masterizer::domain::matching::designFineEq;
using masterizer::domain::matching::FineEqSettings;

TEST_CASE(fine_eq_cuts_a_boosted_band) {
    const int fftSize = 1024;
    const double sampleRate = 48000.0;
    const double binHz = sampleRate / fftSize;
    std::vector<double> source(513, 0.01);
    std::vector<double> reference(513, 0.01);
    for (int bin = 0; bin < 513; ++bin) {
        const double f = bin * binHz;
        if (f > 900.0 && f < 1100.0) {
            source[static_cast<std::size_t>(bin)] = 1.0;
        }
    }
    const auto bands = designFineEq(source, reference, sampleRate, fftSize,
                                    CorrectionBounds{}, FineEqSettings{});
    CHECK(!bands.empty());
    CHECK(bands.front().gainDb < 0.0);
    CHECK(bands.front().frequencyHz > 700.0);
    CHECK(bands.front().frequencyHz < 1400.0);
}

TEST_CASE(fine_eq_is_empty_for_identical_spectra) {
    const std::vector<double> spectrum(513, 0.1);
    const auto bands = designFineEq(spectrum, spectrum, 48000.0, 1024, CorrectionBounds{},
                                    FineEqSettings{});
    CHECK(bands.empty());
}
