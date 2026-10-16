#include "support/FakeDecoder.h"

#include <utility>
#include <string>

namespace masterizer::test {

void FakeDecoder::setResult(const std::string& path,
                            masterizer::domain::audio::DecodedAudio result) {
    results_[path] = std::move(result);
}

masterizer::domain::audio::DecodedAudio FakeDecoder::decode(const std::string& path) const {
    const auto found = results_.find(path);
    if (found != results_.end()) {
        return found->second;
    }
    masterizer::domain::audio::DecodedAudio missing;
    missing.path = path;
    missing.succeeded = false;
    missing.error = "unknown path";
    return missing;
}

}  // namespace masterizer::test
