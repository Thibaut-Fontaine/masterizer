#include "masterizer/domain/analysis/ProfileVersion.h"

namespace masterizer::domain::analysis {

bool isCompatible(const ProfileVersion& a, const ProfileVersion& b) {
    return a.major == b.major;
}

}  // namespace masterizer::domain::analysis
