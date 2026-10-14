#pragma once

#include <cstdint>
#include <string>

namespace masterizer::domain::state {

/// Identifies a specific version of a reference file: path plus file metadata.
struct ReferenceKey {
    std::string path;
    std::uint64_t sizeBytes = 0;
    std::int64_t modifiedTime = 0;
};

}  // namespace masterizer::domain::state
