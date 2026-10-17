#include "masterizer/application/CliOptions.h"
#include "masterizer/application/MasterProfile.h"
#include "masterizer/application/ProcessingChain.h"
#include "masterizer/application/Render.h"
#include "masterizer/application/SourceNotes.h"
#include "masterizer/domain/analysis/SourceAnalyzer.h"
#include "masterizer/domain/audio/AudioBuffer.h"
#include "masterizer/domain/matching/DynamicsPlanner.h"
#include "masterizer/domain/matching/MixAdvisor.h"
#include "masterizer/domain/matching/StereoGuard.h"
#include "masterizer/io/AudioFile.h"

#include <chrono>
#include <cstdio>
#include <future>
#include <string>
#include <vector>

namespace {

void printUsage() {
    std::fprintf(stderr,
                 "usage: masterizer_cli <reference> <source> <output.wav>\n"
                 "  inputs: WAV, FLAC or MP3 (any mix)\n"
                 "  --profile soft|loud|both   delivery profile (default soft)\n"
                 "                              soft = -1.0 dBTP, loud = -0.1 dBTP;\n"
                 "                              both writes <output>.soft.wav and <output>.loud.wav\n"
                 "  --intensity N     Match Intensity 0..2 (default 1)\n"
                 "  --ceiling dB      override the ceiling (single profile only)\n"
                 "  --gain dB         output gain (default 0)\n"
                 "  --ab              also write a level-matched <output>.ab.wav\n"
                 "  --delta           also write <output>.delta.wav (processed - source)\n"
                 "  --timing          print per-phase timings\n"
                 "  --crest-guard / --no-crest-guard  keep dynamics near the reference (default on)\n"
                 "  --<module> / --no-<module>\n"
                 "      module: eq fine-eq mono limiter saturation tape resonance stereo\n"
                 "              transient glue multiband dynamic-eq\n");
}

void applyOverrides(masterizer::application::ChainSettings& chain,
                    const masterizer::application::CliOptions& options) {
    for (const auto& [name, value] : options.moduleOverrides) {
        if (name == "eq") chain.eqEnabled = value;
        else if (name == "fine-eq") chain.fineEqEnabled = value;
        else if (name == "mono") chain.monoBassEnabled = value;
        else if (name == "limiter") chain.limiterEnabled = value;
        else if (name == "saturation") chain.saturationEnabled = value;
        else if (name == "tape") chain.tapeEnabled = value;
        else if (name == "resonance") chain.resonanceEnabled = value;
        else if (name == "stereo") chain.stereoEnabled = value;
        else if (name == "transient") chain.transientEnabled = value;
        else if (name == "glue") chain.glueEnabled = value;
        else if (name == "multiband") chain.multibandEnabled = value;
        else if (name == "dynamic-eq") chain.dynamicEqEnabled = value;
    }
}

bool readOrReport(const std::string& path, const char* label,
                  masterizer::domain::audio::AudioBuffer& out) {
    if (masterizer::io::readAudioFile(path, out)) {
        return true;
    }
    std::fprintf(stderr, "error: could not read %s '%s'\n", label, path.c_str());
    std::fprintf(stderr, "  Supported inputs: WAV, FLAC, MP3.\n");
    return false;
}

}  // namespace

int main(int argc, char** argv) {
    namespace app = masterizer::application;
    namespace ana = masterizer::domain::analysis;
    namespace mat = masterizer::domain::matching;
    using Clock = std::chrono::steady_clock;

    std::vector<std::string> args(argv, argv + argc);
    app::CliOptions options;
    const app::ParseStatus status = app::parseCliOptions(args, options);
    if (status == app::ParseStatus::Help) {
        printUsage();
        return 0;
    }
    if (status == app::ParseStatus::Error) {
        printUsage();
        return 2;
    }

    const Clock::time_point t0 = Clock::now();
    masterizer::domain::audio::AudioBuffer reference;
    masterizer::domain::audio::AudioBuffer source;
    if (!readOrReport(options.referencePath, "reference", reference) ||
        !readOrReport(options.sourcePath, "source", source)) {
        return 1;
    }
    const Clock::time_point tRead = Clock::now();

    const ana::AnalysisSettings settings{2048, 1024};
    auto referenceFuture = std::async(std::launch::async, [&] {
        return ana::analyzeSource(reference, settings);
    });
    auto sourceFuture = std::async(std::launch::async, [&] {
        return ana::analyzeSource(source, settings);
    });
    const ana::AnalysisProfile referenceProfile = referenceFuture.get();
    const ana::AnalysisProfile sourceProfile = sourceFuture.get();
    const Clock::time_point tAnalyze = Clock::now();

    const mat::MixFeedback feedback =
        mat::adviseFeedback(sourceProfile, referenceProfile, mat::AdvisorRuleset{});
    const std::string sourceNotes = app::sourceNotesBlock(feedback.suggestions);
    if (!sourceNotes.empty()) {
        std::printf("%s", sourceNotes.c_str());
    }

    app::ChainSettings chain;
    chain.matchIntensity = options.matchIntensity;
    chain.outputGainDb = options.outputGainDb;
    const mat::DynamicsPlan dynamics = mat::planDynamics(sourceProfile, referenceProfile, 0.0);
    chain.glueEnabled = dynamics.glueEnabled;
    chain.glue = dynamics.glue;
    chain.multibandEnabled = dynamics.multibandEnabled;
    chain.dynamicEqEnabled = dynamics.dynamicEqEnabled;
    chain.dynamicEq = dynamics.dynamicEq;
    chain.tapeEnabled = true;  // very subtle tape at the end, toggle with --no-tape
    chain.tape = {1.0, 0.4, -0.1, 0.5};
    applyOverrides(chain, options);
    if (!mat::stereoMatchAllowed(referenceProfile.stereo.width)) {
        chain.stereoEnabled = false;  // near-mono reference: do not chase width
    }

    const bool limiterEnabled = chain.limiterEnabled;
    chain.limiterEnabled = false;  // the loudness targeter applies the limiter at the end
    const app::ChainResult processed = app::process(source, sourceProfile, referenceProfile, chain);
    const Clock::time_point tProcess = Clock::now();

    const app::RenderJob job{referenceProfile, sourceProfile, source, processed, options,
                             limiterEnabled};
    bool ok = true;
    if (options.both) {
        ok = app::renderMaster(job, app::MasterProfile::Soft,
                               app::masterOutputPath(options.outputPath,
                                                     app::MasterProfile::Soft)) &&
             ok;
        ok = app::renderMaster(job, app::MasterProfile::Loud,
                               app::masterOutputPath(options.outputPath,
                                                     app::MasterProfile::Loud)) &&
             ok;
    } else {
        ok = app::renderMaster(job, options.profile, options.outputPath);
    }
    const Clock::time_point tWrite = Clock::now();
    if (!ok) {
        return 1;
    }

    if (options.timing) {
        const auto ms = [](Clock::time_point a, Clock::time_point b) {
            return std::chrono::duration<double, std::milli>(b - a).count();
        };
        std::printf("timing(ms): read=%.0f analyze=%.0f process=%.0f render=%.0f total=%.0f\n",
                    ms(t0, tRead), ms(tRead, tAnalyze), ms(tAnalyze, tProcess),
                    ms(tProcess, tWrite), ms(t0, tWrite));
    }
    return 0;
}
