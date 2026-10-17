#pragma once

#include "masterizer/application/CliOptions.h"
#include "masterizer/application/LoudnessTargeter.h"
#include "masterizer/application/MasterProfile.h"
#include "masterizer/application/ProcessingChain.h"
#include "masterizer/domain/analysis/AnalysisProfile.h"
#include "masterizer/domain/audio/AudioBuffer.h"

#include <string>

namespace masterizer::application {

/// Everything needed to render one (or both) delivery profiles without
/// re-analysing or re-running the chain.
struct RenderJob {
    const domain::analysis::AnalysisProfile& reference;
    const domain::analysis::AnalysisProfile& source;
    const domain::audio::AudioBuffer& sourceAudio;
    const ChainResult& processed;
    const CliOptions& options;
    bool limiterEnabled = true;
};

/// Applies loudness targeting for `profile`, enforces the ceiling, writes the
/// WAV (plus optional A/B and delta) and prints the summary. Returns false only
/// if the file could not be written.
bool renderMaster(const RenderJob& job, MasterProfile profile, const std::string& outputPath);

/// Inserts the profile tag before the extension: "out.wav" -> "out.loud.wav".
std::string masterOutputPath(const std::string& base, MasterProfile profile);

}  // namespace masterizer::application
