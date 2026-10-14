#pragma once

#include "masterizer/domain/audio/DecodedAudio.h"

#include <string>

namespace masterizer::domain::audio {

/// Port for decoding an audio file. Implemented by the JUCE adapter in the
/// plugin layer; faked in tests so the domain stays free of JUCE.
class IAudioDecoder {
public:
    virtual ~IAudioDecoder() = default;
    virtual DecodedAudio decode(const std::string& path) const = 0;
};

}  // namespace masterizer::domain::audio
