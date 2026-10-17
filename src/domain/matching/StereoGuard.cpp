#include "masterizer/domain/matching/StereoGuard.h"

namespace masterizer::domain::matching {

bool stereoMatchAllowed(double referenceWidth, double minimumWidth) {
    return referenceWidth >= minimumWidth;
}

}  // namespace masterizer::domain::matching
