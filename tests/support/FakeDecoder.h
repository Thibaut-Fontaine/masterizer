#pragma once

#include "masterizer/domain/audio/IAudioDecoder.h"

#include <map>
#include <string>

namespace masterizer::test {

/// In-memory IAudioDecoder for tests: returns preconfigured results by path.
class FakeDecoder : public masterizer::domain::audio::IAudioDecoder {
public:
    void setResult(const std::string& path, masterizer::domain::audio::DecodedAudio result);
    masterizer::domain::audio::DecodedAudio decode(const std::string& path) const override;

private:
    std::map<std::string, masterizer::domain::audio::DecodedAudio> results_;
};

}  // namespace masterizer::test
