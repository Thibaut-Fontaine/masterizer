#include "masterizer/domain/analysis/BandMapping.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::analysis::bandIndexForFrequency;

TEST_CASE(band_index_maps_known_frequencies) {
    CHECK(bandIndexForFrequency(20.0) == 0);    // Sub
    CHECK(bandIndexForFrequency(60.0) == 1);    // Low
    CHECK(bandIndexForFrequency(1000.0) == 5);  // UpperMid
    CHECK(bandIndexForFrequency(20000.0) == 9); // VeryHigh (open above)
}

TEST_CASE(band_index_rejects_invalid_frequencies) {
    CHECK(bandIndexForFrequency(-1.0) == -1);
}
