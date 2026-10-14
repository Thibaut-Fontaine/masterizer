#pragma once

namespace masterizer::domain::state {

/// Lifecycle of the loaded reference file.
enum class ReferenceStatus {
    Unloaded,
    Loading,
    Ready,
    Stale,
    Failed,
};

}  // namespace masterizer::domain::state
