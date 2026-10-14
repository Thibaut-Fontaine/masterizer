#pragma once

#include "masterizer/domain/state/ReferenceKey.h"

#include <string>

namespace masterizer::domain::state {

/// Stable cache id combining path, size and modification time.
std::string cacheId(const ReferenceKey& key);

/// True when two keys identify the same file version.
bool sameReference(const ReferenceKey& a, const ReferenceKey& b);

}  // namespace masterizer::domain::state
