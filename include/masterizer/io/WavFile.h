#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"

#include <string>

namespace masterizer::io {

/// Reads a 16-bit PCM or 32-bit float WAV into planar float channels.
/// Returns false on any malformed or unsupported file.
bool readWav(const std::string& path, domain::audio::AudioBuffer& out);

/// Writes planar float channels as a 16-bit PCM WAV.
bool writeWav(const std::string& path, const domain::audio::AudioBuffer& buffer);

}  // namespace masterizer::io
