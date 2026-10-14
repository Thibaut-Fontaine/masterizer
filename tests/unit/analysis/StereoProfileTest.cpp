#include "masterizer/domain/analysis/StereoProfile.h"

#include "masterizer/domain/audio/AudioBuffer.h"
#include "support/Buffers.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/Stereo.h"
#include "support/TestRegistry.h"

using masterizer::domain::analysis::analyzeStereo;
using masterizer::domain::audio::AudioBuffer;
using masterizer::test::antiPhase;
using masterizer::test::sine;
using masterizer::test::stereoBuffer;
using masterizer::test::wideStereo;

TEST_CASE(identical_channels_have_no_side_energy) {
    const auto mono = sine(8192, 1000.0, 48000.0, 0.5f);
    const auto profile = analyzeStereo(stereoBuffer(mono, mono, 48000.0), 1024, 256);
    CHECK_NEAR(profile.correlation, 1.0, 1e-6);
    CHECK_NEAR(profile.width, 0.0, 1e-9);
}

TEST_CASE(anti_phase_channels_are_all_side) {
    const auto pair = antiPhase(8192, 1000.0, 48000.0, 0.5f);
    AudioBuffer buffer;
    buffer.channels = {pair.left, pair.right};
    buffer.sampleRate = 48000.0;
    const auto profile = analyzeStereo(buffer, 1024, 256);
    CHECK_NEAR(profile.correlation, -1.0, 1e-6);
    CHECK(profile.width > 0.99);
}

TEST_CASE(uncorrelated_channels_are_wide) {
    const auto pair = wideStereo(8192, 3u, 0.5f);
    AudioBuffer buffer;
    buffer.channels = {pair.left, pair.right};
    buffer.sampleRate = 48000.0;
    const auto profile = analyzeStereo(buffer, 1024, 256);
    CHECK(profile.correlation > -0.1 && profile.correlation < 0.1);
    CHECK(profile.width > 0.3);
}
