#include "masterizer/domain/common/OctaveBands.h"

#include <cmath>
#include <limits>

namespace masterizer::domain {
namespace {

constexpr int kBandCount = 10;
constexpr double kEdges[kBandCount] = {
    0.0, 45.0, 90.0, 180.0, 360.0, 720.0, 1440.0, 2880.0, 5760.0, 11520.0,
};

bool isValid(OctaveBand band) {
    const int index = static_cast<int>(band);
    return index >= 0 && index < kBandCount;
}

}  // namespace

int bandCount() {
    return kBandCount;
}

double lowerEdgeHz(OctaveBand band) {
    return isValid(band) ? kEdges[static_cast<int>(band)] : 0.0;
}

double upperEdgeHz(OctaveBand band) {
    if (!isValid(band)) {
        return 0.0;
    }
    const int index = static_cast<int>(band);
    if (index == kBandCount - 1) {
        return std::numeric_limits<double>::infinity();
    }
    return kEdges[index + 1];
}

double centreHz(OctaveBand band) {
    const double low = lowerEdgeHz(band);
    const double high = upperEdgeHz(band);
    if (low <= 0.0 || !std::isfinite(high)) {
        return 0.0;
    }
    return std::sqrt(low * high);
}

}  // namespace masterizer::domain
