#include "masterizer/io/AudioFile.h"

#include "masterizer/domain/dsp/Finite.h"
#include "masterizer/domain/dsp/Rms.h"
#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <string>

using masterizer::domain::audio::AudioBuffer;
using masterizer::domain::dsp::allFinite;
using masterizer::domain::dsp::rms;
using masterizer::io::readAudioFile;

namespace {

std::string fixture(const char* name) {
    return std::string(MASTERIZER_FIXTURES_DIR) + "/" + name;
}

}  // namespace

TEST_CASE(reads_a_flac_file_losslessly) {
    AudioBuffer buffer;
    CHECK(readAudioFile(fixture("sine_1k.flac"), buffer));
    CHECK(buffer.channels.size() == 1u);
    CHECK_NEAR(buffer.sampleRate, 48000.0, 1e-9);
    CHECK(buffer.channels[0].size() == 12000u);
    CHECK_NEAR(rms(buffer.channels[0].data(), 12000), 0.3536, 0.01);
}

TEST_CASE(reads_an_mp3_file) {
    AudioBuffer buffer;
    CHECK(readAudioFile(fixture("sine_1k.mp3"), buffer));
    CHECK(buffer.channels.size() == 1u);
    CHECK_NEAR(buffer.sampleRate, 48000.0, 1e-9);
    CHECK(buffer.channels[0].size() > 10000u);
    const int frames = static_cast<int>(buffer.channels[0].size());
    CHECK(allFinite(buffer.channels[0].data(), frames));
    CHECK_NEAR(rms(buffer.channels[0].data(), frames), 0.3536, 0.05);
}

TEST_CASE(an_unreadable_file_is_rejected) {
    AudioBuffer buffer;
    CHECK(!readAudioFile("/nonexistent/nope.mp3", buffer));
}
