#include "masterizer/domain/audio/AudioBufferOps.h"

#include "support/Buffers.h"
#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <vector>

using masterizer::domain::audio::AudioBuffer;
using masterizer::domain::audio::channelCount;
using masterizer::domain::audio::frameCount;
using masterizer::test::monoBuffer;
using masterizer::test::stereoBuffer;

TEST_CASE(empty_buffer_has_no_channels_or_frames) {
    const AudioBuffer buffer;
    CHECK(channelCount(buffer) == 0);
    CHECK(frameCount(buffer) == 0);
}

TEST_CASE(mono_buffer_reports_one_channel) {
    const auto buffer = monoBuffer({0.1f, 0.2f, 0.3f}, 48000.0);
    CHECK(channelCount(buffer) == 1);
    CHECK(frameCount(buffer) == 3);
    CHECK_NEAR(buffer.sampleRate, 48000.0, 1e-9);
}

TEST_CASE(stereo_buffer_reports_frames_of_first_channel) {
    const auto buffer = stereoBuffer({1.0f, 2.0f}, {-1.0f, -2.0f}, 44100.0);
    CHECK(channelCount(buffer) == 2);
    CHECK(frameCount(buffer) == 2);
    CHECK_NEAR(buffer.channels[1][0], -1.0, 1e-9);
}
