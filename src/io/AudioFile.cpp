#include "masterizer/io/AudioFile.h"

#include "masterizer/io/WavFile.h"

#include <cstddef>
#include <string>
#include <vector>

extern "C" {
#include "dr_libs/dr_flac.h"
#include "dr_libs/dr_mp3.h"
}

namespace masterizer::io {
namespace {

void interleavedToPlanar(const std::vector<float>& interleaved, int channels, int frames,
                         domain::audio::AudioBuffer& out) {
    out.channels.assign(static_cast<std::size_t>(channels),
                        std::vector<float>(static_cast<std::size_t>(frames), 0.0f));
    for (int frame = 0; frame < frames; ++frame) {
        for (int channel = 0; channel < channels; ++channel) {
            out.channels[static_cast<std::size_t>(channel)][static_cast<std::size_t>(frame)] =
                interleaved[static_cast<std::size_t>(frame) * channels + channel];
        }
    }
}

bool readFlac(const std::string& path, domain::audio::AudioBuffer& out) {
    drflac* flac = drflac_open_file(path.c_str(), nullptr);
    if (flac == nullptr) {
        return false;
    }
    const int channels = static_cast<int>(flac->channels);
    const int frames = static_cast<int>(flac->totalPCMFrameCount);
    out.sampleRate = static_cast<double>(flac->sampleRate);
    std::vector<float> interleaved(static_cast<std::size_t>(frames) * channels, 0.0f);
    if (frames > 0) {
        drflac_read_pcm_frames_f32(flac, static_cast<drflac_uint64>(frames), interleaved.data());
    }
    drflac_close(flac);
    interleavedToPlanar(interleaved, channels, frames, out);
    return frames > 0;
}

bool readMp3(const std::string& path, domain::audio::AudioBuffer& out) {
    drmp3 mp3;
    if (drmp3_init_file(&mp3, path.c_str(), nullptr) == 0) {
        return false;
    }
    const int channels = static_cast<int>(mp3.channels);
    const int frames = static_cast<int>(drmp3_get_pcm_frame_count(&mp3));
    out.sampleRate = static_cast<double>(mp3.sampleRate);
    std::vector<float> interleaved(static_cast<std::size_t>(frames) * channels, 0.0f);
    if (frames > 0) {
        drmp3_seek_to_pcm_frame(&mp3, 0);
        drmp3_read_pcm_frames_f32(&mp3, static_cast<drmp3_uint64>(frames), interleaved.data());
    }
    drmp3_uninit(&mp3);
    interleavedToPlanar(interleaved, channels, frames, out);
    return frames > 0;
}

}  // namespace

bool readAudioFile(const std::string& path, domain::audio::AudioBuffer& out) {
    return readWav(path, out) || readFlac(path, out) || readMp3(path, out);
}

}  // namespace masterizer::io
