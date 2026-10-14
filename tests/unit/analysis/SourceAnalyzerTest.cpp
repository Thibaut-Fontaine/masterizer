#include "masterizer/domain/analysis/SourceAnalyzer.h"

#include "masterizer/domain/analysis/OctaveProfile.h"
#include "masterizer/domain/analysis/Stft.h"
#include "masterizer/domain/audio/AudioBufferOps.h"
#include "masterizer/domain/common/OctaveBands.h"
#include "support/Buffers.h"
#include "support/Checks.h"
#include "support/Signals.h"
#include "support/TestRegistry.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

using masterizer::domain::analysis::AnalysisSettings;
using masterizer::domain::analysis::analyzeSource;
using masterizer::domain::analysis::isCompatible;
using masterizer::domain::analysis::kCurrentProfileVersion;
using masterizer::domain::bandCount;
using masterizer::test::monoBuffer;
using masterizer::test::silence;
using masterizer::test::sine;

namespace {

constexpr AnalysisSettings kSettings{1024, 256};

}  // namespace

TEST_CASE(source_profile_has_expected_shapes_and_version) {
    const auto data = sine(8192, 1000.0, 48000.0, 0.5f);
    const auto profile = analyzeSource(monoBuffer(data, 48000.0), kSettings);
    CHECK(profile.octaveEnergy.size() == static_cast<std::size_t>(bandCount()));
    CHECK(profile.spectrum.size() == 513u);
    CHECK(isCompatible(profile.version, kCurrentProfileVersion));
}

TEST_CASE(source_profile_locates_a_one_khz_tone) {
    const auto data = sine(8192, 1000.0, 48000.0, 0.5f);
    const auto profile = analyzeSource(monoBuffer(data, 48000.0), kSettings);
    const auto peak = std::max_element(profile.octaveEnergy.begin(),
                                       profile.octaveEnergy.end());
    CHECK(std::distance(profile.octaveEnergy.begin(), peak) == 5);
    CHECK(profile.loudness.samplePeakDb > -7.0);
}

TEST_CASE(source_profile_of_silence_is_empty_and_quiet) {
    const auto data = silence(8192);
    const auto profile = analyzeSource(monoBuffer(data, 48000.0), kSettings);
    CHECK(std::isinf(profile.loudness.integratedLufs));
    CHECK(profile.envelope.percussiveProminence == 0.0);
    for (double energy : profile.octaveEnergy) {
        CHECK(energy == 0.0);
    }
}
