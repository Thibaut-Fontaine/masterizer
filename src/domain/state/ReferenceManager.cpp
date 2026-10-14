#include "masterizer/domain/state/ReferenceManager.h"

#include "masterizer/domain/audio/AudioBufferOps.h"
#include "masterizer/domain/audio/DecodedAudio.h"
#include "masterizer/domain/state/ReferenceKeyOps.h"

namespace masterizer::domain::state {
namespace {

bool isValidDecode(const audio::DecodedAudio& decoded) {
    return decoded.succeeded && audio::channelCount(decoded.buffer) >= 1 &&
           audio::frameCount(decoded.buffer) > 0 && decoded.buffer.sampleRate > 0.0;
}

}  // namespace

ReferenceManager::ReferenceManager(const audio::IAudioDecoder& decoder) : decoder_(decoder) {}

const ReferenceState& ReferenceManager::load(const std::string& path, const ReferenceKey& key) {
    state_.key = key;
    state_.status = ReferenceStatus::Loading;
    state_.buffer = {};
    state_.error.clear();

    const audio::DecodedAudio decoded = decoder_.decode(path);
    if (!isValidDecode(decoded)) {
        state_.status = ReferenceStatus::Failed;
        state_.buffer = {};
        state_.error = decoded.error.empty() ? "decode failed" : decoded.error;
        return state_;
    }
    state_.status = ReferenceStatus::Ready;
    state_.buffer = decoded.buffer;
    state_.error.clear();
    return state_;
}

const ReferenceState& ReferenceManager::unload() {
    state_ = ReferenceState{};
    return state_;
}

bool ReferenceManager::isStale(const ReferenceKey& key) const {
    return state_.status == ReferenceStatus::Ready && !sameReference(state_.key, key);
}

}  // namespace masterizer::domain::state
