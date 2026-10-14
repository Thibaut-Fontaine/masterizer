#include "masterizer/domain/state/ProfileCache.h"

#include "masterizer/domain/analysis/ProfileVersion.h"
#include "masterizer/domain/state/ReferenceKeyOps.h"

#include <utility>

namespace masterizer::domain::state {

void ProfileCache::store(const ReferenceKey& key,
                         std::shared_ptr<const analysis::AnalysisProfile> profile) {
    entries_[cacheId(key)] = std::move(profile);
}

std::shared_ptr<const analysis::AnalysisProfile> ProfileCache::lookup(
    const ReferenceKey& key) const {
    const auto found = entries_.find(cacheId(key));
    if (found == entries_.end() || !found->second) {
        return nullptr;
    }
    if (!analysis::isCompatible(found->second->version, analysis::kCurrentProfileVersion)) {
        return nullptr;
    }
    return found->second;
}

void ProfileCache::clear() {
    entries_.clear();
}

int ProfileCache::size() const {
    return static_cast<int>(entries_.size());
}

}  // namespace masterizer::domain::state
