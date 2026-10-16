#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"

#include <string>

namespace masterizer::io {

/// Reads a WAV (16-bit PCM / 32-bit float), FLAC or MP3 file depending on the
/// content. Returns false if none of the decoders accept the file.
bool readAudioFile(const std::string& path, domain::audio::AudioBuffer& out);

}  // namespace masterizer::io
