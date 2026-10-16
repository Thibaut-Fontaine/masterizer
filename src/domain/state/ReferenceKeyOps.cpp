#include "masterizer/domain/state/ReferenceKeyOps.h"
#include <string>

namespace masterizer::domain::state {

std::string cacheId(const ReferenceKey& key) {
    return key.path + "#" + std::to_string(key.sizeBytes) + "#" +
           std::to_string(key.modifiedTime);
}

bool sameReference(const ReferenceKey& a, const ReferenceKey& b) {
    return a.path == b.path && a.sizeBytes == b.sizeBytes && a.modifiedTime == b.modifiedTime;
}

}  // namespace masterizer::domain::state
