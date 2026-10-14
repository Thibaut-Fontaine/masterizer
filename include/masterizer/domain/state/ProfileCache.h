#pragma once

#include "masterizer/domain/analysis/AnalysisProfile.h"
#include "masterizer/domain/state/ReferenceKey.h"

#include <map>
#include <memory>
#include <string>

namespace masterizer::domain::state {

/// Stores analysis profiles keyed by reference identity. A stored profile is
/// only returned while its ProfileVersion stays compatible with the current one.
class ProfileCache {
public:
    void store(const ReferenceKey& key, std::shared_ptr<const analysis::AnalysisProfile> profile);
    std::shared_ptr<const analysis::AnalysisProfile> lookup(const ReferenceKey& key) const;
    void clear();
    int size() const;

private:
    std::map<std::string, std::shared_ptr<const analysis::AnalysisProfile>> entries_;
};

}  // namespace masterizer::domain::state
