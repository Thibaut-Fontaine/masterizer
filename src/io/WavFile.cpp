#include "masterizer/io/WavFile.h"

#include "io/WavBytes.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>

namespace masterizer::io {
namespace {

struct WavFormat {
    std::uint16_t audioFormat = 0;
    std::uint16_t channels = 0;
    std::uint16_t bits = 0;
    std::uint32_t sampleRate = 0;
};

bool locate(const std::vector<unsigned char>& bytes, WavFormat& format, const unsigned char*& data,
            std::uint32_t& dataSize) {
    if (bytes.size() < 44u || std::memcmp(bytes.data(), "RIFF", 4) != 0 ||
        std::memcmp(bytes.data() + 8, "WAVE", 4) != 0) {
        return false;
    }
    data = nullptr;
    dataSize = 0;
    std::size_t pos = 12;
    while (pos + 8u <= bytes.size()) {
        const unsigned char* id = bytes.data() + pos;
        const std::uint32_t size = readU32(bytes.data() + pos + 4);
        pos += 8;
        if (pos + size > bytes.size()) {
            break;
        }
        if (std::memcmp(id, "fmt ", 4) == 0 && size >= 16u) {
            format.audioFormat = readU16(bytes.data() + pos);
            format.channels = readU16(bytes.data() + pos + 2);
            format.sampleRate = readU32(bytes.data() + pos + 4);
            format.bits = readU16(bytes.data() + pos + 14);
        } else if (std::memcmp(id, "data", 4) == 0) {
            data = bytes.data() + pos;
            dataSize = size;
        }
        pos += size + (size & 1u);
    }
    return data != nullptr && format.channels > 0u;
}

void fillChannels(const unsigned char* data, std::uint32_t dataSize, const WavFormat& format,
                  bool isFloat, domain::audio::AudioBuffer& out) {
    const int bytesPerSample = isFloat ? 4 : 2;
    const std::uint32_t frames =
        dataSize / (static_cast<std::uint32_t>(bytesPerSample) * format.channels);
    out.channels.assign(format.channels, std::vector<float>(frames, 0.0f));
    out.sampleRate = format.sampleRate;
    for (std::uint32_t frame = 0; frame < frames; ++frame) {
        for (std::uint16_t channel = 0; channel < format.channels; ++channel) {
            const std::size_t offset =
                (static_cast<std::size_t>(frame) * format.channels + channel) *
                static_cast<std::size_t>(bytesPerSample);
            float value = 0.0f;
            if (isFloat) {
                std::memcpy(&value, data + offset, sizeof(float));
            } else {
                value = static_cast<float>(static_cast<std::int16_t>(readU16(data + offset))) /
                        32768.0f;
            }
            out.channels[channel][frame] = value;
        }
    }
}

}  // namespace

bool readWav(const std::string& path, domain::audio::AudioBuffer& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    const std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)),
                                           std::istreambuf_iterator<char>());
    WavFormat format;
    const unsigned char* data = nullptr;
    std::uint32_t dataSize = 0;
    if (!locate(bytes, format, data, dataSize)) {
        return false;
    }
    const bool isFloat = format.audioFormat == 3u && format.bits == 32u;
    const bool isPcm16 = format.audioFormat == 1u && format.bits == 16u;
    if (!isFloat && !isPcm16) {
        return false;
    }
    fillChannels(data, dataSize, format, isFloat, out);
    return true;
}

bool writeWav(const std::string& path, const domain::audio::AudioBuffer& buffer) {
    if (buffer.channels.empty() || buffer.channels[0].empty()) {
        return false;
    }
    const std::uint16_t channels = static_cast<std::uint16_t>(buffer.channels.size());
    const std::uint32_t frames = static_cast<std::uint32_t>(buffer.channels[0].size());
    const std::uint32_t dataBytes = frames * channels * 2u;
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    unsigned char header[44] = {};
    std::memcpy(header, "RIFF", 4);
    writeU32(header + 4, 36u + dataBytes);
    std::memcpy(header + 8, "WAVE", 4);
    std::memcpy(header + 12, "fmt ", 4);
    writeU32(header + 16, 16u);
    header[20] = 1;  // PCM
    header[22] = static_cast<unsigned char>(channels & 0xFF);
    header[23] = static_cast<unsigned char>((channels >> 8) & 0xFF);
    writeU32(header + 24, static_cast<std::uint32_t>(buffer.sampleRate));
    writeU32(header + 28, static_cast<std::uint32_t>(buffer.sampleRate) * channels * 2u);
    header[32] = static_cast<unsigned char>((channels * 2u) & 0xFF);
    header[34] = 16;  // bits per sample
    std::memcpy(header + 36, "data", 4);
    writeU32(header + 40, dataBytes);
    file.write(reinterpret_cast<const char*>(header), 44);
    for (std::uint32_t frame = 0; frame < frames; ++frame) {
        for (std::uint16_t channel = 0; channel < channels; ++channel) {
            const double clamped =
                std::clamp(static_cast<double>(buffer.channels[channel][frame]), -1.0, 1.0);
            const std::int16_t sample = static_cast<std::int16_t>(std::lround(clamped * 32767.0));
            const unsigned char pair[2] = {static_cast<unsigned char>(sample & 0xFF),
                                           static_cast<unsigned char>((sample >> 8) & 0xFF)};
            file.write(reinterpret_cast<const char*>(pair), 2);
        }
    }
    return static_cast<bool>(file);
}

}  // namespace masterizer::io
