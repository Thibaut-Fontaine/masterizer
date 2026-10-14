#include "masterizer/domain/analysis/BandMapping.h"

#include "masterizer/domain/common/OctaveBands.h"

namespace masterizer::domain::analysis {

int bandIndexForFrequency(double frequency) {
    for (int index = 0; index < domain::bandCount(); ++index) {
        const auto band = static_cast<domain::OctaveBand>(index);
        if (frequency >= domain::lowerEdgeHz(band) && frequency < domain::upperEdgeHz(band)) {
            return index;
        }
    }
    return -1;
}

}  // namespace masterizer::domain::analysis
