#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"
#include "masterizer/domain/audio/IAudioDecoder.h"
#include "masterizer/domain/state/ReferenceKey.h"
#include "masterizer/domain/state/ReferenceStatus.h"

#include <string>

namespace masterizer::domain::state {

/// Current reference loading state, including the decoded audio when ready.
struct ReferenceState {
    ReferenceStatus status = ReferenceStatus::Unloaded;
    ReferenceKey key;
    audio::AudioBuffer buffer;
    std::string error;
};

/// Loads and validates the reference through an injected decoder port.
/// All heavy work happens off the audio thread in the adapter that calls this.
class ReferenceManager {
public:
    explicit ReferenceManager(const audio::IAudioDecoder& decoder);
    const ReferenceState& load(const std::string& path, const ReferenceKey& key);
    const ReferenceState& unload();
    bool isStale(const ReferenceKey& key) const;

private:
    const audio::IAudioDecoder& decoder_;
    ReferenceState state_;
};

}  // namespace masterizer::domain::state
