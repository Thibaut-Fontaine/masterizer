#include "masterizer/io/WavFile.h"

#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <filesystem>
#include <string>
#include <vector>
#include <cstddef>

using masterizer::domain::audio::AudioBuffer;
using masterizer::io::readWav;
using masterizer::io::writeWav;

namespace {

std::string tempPath(const char* name) {
    return (std::filesystem::temp_directory_path() / name).string();
}

}  // namespace

TEST_CASE(mono_wav_round_trips_within_quantisation) {
    AudioBuffer buffer;
    buffer.channels = {{0.0f, 0.5f, -0.5f, 1.0f, -1.0f}};
    buffer.sampleRate = 44100.0;
    const std::string path = tempPath("masterizer_mono.wav");
    CHECK(writeWav(path, buffer));
    AudioBuffer loaded;
    CHECK(readWav(path, loaded));
    CHECK(loaded.channels.size() == 1u);
    CHECK_NEAR(loaded.sampleRate, 44100.0, 1e-9);
    for (std::size_t i = 0; i < buffer.channels[0].size(); ++i) {
        CHECK_NEAR(loaded.channels[0][i], buffer.channels[0][i], 2.0 / 32767.0);
    }
}

TEST_CASE(stereo_wav_round_trips_channel_count) {
    AudioBuffer buffer;
    buffer.channels = {{0.1f, 0.2f}, {-0.1f, -0.2f}};
    buffer.sampleRate = 48000.0;
    const std::string path = tempPath("masterizer_stereo.wav");
    CHECK(writeWav(path, buffer));
    AudioBuffer loaded;
    CHECK(readWav(path, loaded));
    CHECK(loaded.channels.size() == 2u);
    CHECK(loaded.channels[0].size() == 2u);
}

TEST_CASE(reading_a_missing_file_fails_cleanly) {
    AudioBuffer loaded;
    CHECK(!readWav("/nonexistent/does-not-exist.wav", loaded));
}

TEST_CASE(reads_a_24_bit_pcm_wav) {
    AudioBuffer buffer;
    REQUIRE(readWav(std::string(MASTERIZER_FIXTURES_DIR) + "/sine_1k_24.wav", buffer));
    REQUIRE(buffer.channels.size() == 1u);
    CHECK_NEAR(buffer.sampleRate, 48000.0, 1e-9);
    CHECK(buffer.channels[0].size() == 12000u);
    CHECK_NEAR(masterizer::domain::dsp::rms(buffer.channels[0].data(), 12000), 0.3536, 0.01);
}
