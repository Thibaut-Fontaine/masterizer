#include "masterizer/application/CliOptions.h"
#include "masterizer/application/MonitoringController.h"
#include "masterizer/application/OutputValidator.h"
#include "masterizer/application/ProcessingChain.h"
#include "masterizer/application/TruePeakGuard.h"
#include "masterizer/domain/analysis/LoudnessProfile.h"
#include "masterizer/domain/analysis/SourceAnalyzer.h"
#include "masterizer/domain/audio/AudioBuffer.h"
#include "masterizer/domain/dsp/GainStage.h"
#include "masterizer/domain/matching/DynamicsPlanner.h"
#include "masterizer/io/WavFile.h"

#include <cstdio>
#include <string>
#include <vector>
#include <cstddef>

namespace {

void printUsage() {
    std::fprintf(stderr,
                 "usage: masterizer_cli <reference.wav> <source.wav> <output.wav>\n"
                 "  --intensity N     Match Intensity 0..2 (default 1)\n"
                 "  --ceiling dB      True Peak ceiling (default -1)\n"
                 "  --gain dB         output gain (default 0)\n"
                 "  --ab              also write a level-matched <output>.ab.wav\n"
                 "  --delta           also write <output>.delta.wav (processed - source)\n"
                 "  --<module> / --no-<module>\n"
                 "      module: eq mono limiter saturation resonance stereo transient\n"
                 "              glue multiband dynamic-eq\n");
}

void applyOverrides(masterizer::application::ChainSettings& chain,
                    const masterizer::application::CliOptions& options) {
    for (const auto& [name, value] : options.moduleOverrides) {
        if (name == "eq") chain.eqEnabled = value;
        else if (name == "mono") chain.monoBassEnabled = value;
        else if (name == "limiter") chain.limiterEnabled = value;
        else if (name == "saturation") chain.saturationEnabled = value;
        else if (name == "resonance") chain.resonanceEnabled = value;
        else if (name == "stereo") chain.stereoEnabled = value;
        else if (name == "transient") chain.transientEnabled = value;
        else if (name == "glue") chain.glueEnabled = value;
        else if (name == "multiband") chain.multibandEnabled = value;
        else if (name == "dynamic-eq") chain.dynamicEqEnabled = value;
    }
}

std::string auxPath(const std::string& output, const char* tag) {
    const std::size_t dot = output.find_last_of('.');
    if (dot == std::string::npos) {
        return output + tag;
    }
    return output.substr(0, dot) + tag + output.substr(dot);
}

bool readOrReport(const std::string& path, const char* label,
                  masterizer::domain::audio::AudioBuffer& out) {
    if (masterizer::io::readWav(path, out)) {
        return true;
    }
    std::fprintf(stderr, "error: could not read %s '%s'\n", label, path.c_str());
    std::fprintf(stderr, "  This tool reads WAV only (16-bit PCM or 32-bit float).\n");
    std::fprintf(stderr,
                 "  Convert first, e.g.:  ffmpeg -i \"%s\" -c:a pcm_s16le out.wav\n",
                 path.c_str());
    return false;
}

}  // namespace

int main(int argc, char** argv) {
    namespace app = masterizer::application;
    namespace ana = masterizer::domain::analysis;
    namespace mat = masterizer::domain::matching;

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

    masterizer::domain::audio::AudioBuffer reference;
    masterizer::domain::audio::AudioBuffer source;
    if (!readOrReport(options.referencePath, "reference", reference) ||
        !readOrReport(options.sourcePath, "source", source)) {
        return 1;
    }

    const ana::AnalysisSettings settings;
    const ana::AnalysisProfile referenceProfile = ana::analyzeSource(reference, settings);
    const ana::AnalysisProfile sourceProfile = ana::analyzeSource(source, settings);

    app::ChainSettings chain;
    chain.matchIntensity = options.matchIntensity;
    chain.outputGainDb = options.outputGainDb;
    chain.limiter.ceilingDb = options.ceilingDb;
    chain.resonanceEnabled = true;
    const mat::DynamicsPlan dynamics = mat::planDynamics(sourceProfile, referenceProfile, 0.0);
    chain.glueEnabled = dynamics.glueEnabled;
    chain.glue = dynamics.glue;
    chain.multibandEnabled = dynamics.multibandEnabled;
    applyOverrides(chain, options);

    app::ChainResult result = app::process(source, sourceProfile, referenceProfile, chain);
    app::enforceTruePeak(result.buffer, chain.limiter.ceilingDb);
    const app::OutputReport report = app::validateOutput(
        result.buffer, referenceProfile.loudness.integratedLufs, chain.limiter.ceilingDb);

    if (!masterizer::io::writeWav(options.outputPath, result.buffer)) {
        std::fprintf(stderr, "error: could not write '%s'\n", options.outputPath.c_str());
        return 1;
    }
    if (options.writeAb) {
        masterizer::domain::audio::AudioBuffer ab = source;
        const double sourceLufs = ana::analyzeLoudness(source).integratedLufs;
        const double gainDb = app::loudnessMatchGainDb(sourceLufs, report.integratedLufs);
        for (std::vector<float>& channel : ab.channels) {
            masterizer::domain::dsp::applyGain(channel.data(),
                                               static_cast<int>(channel.size()), gainDb);
        }
        masterizer::io::writeWav(auxPath(options.outputPath, ".ab"), ab);
    }
    if (options.writeDelta) {
        masterizer::io::writeWav(auxPath(options.outputPath, ".delta"),
                                 app::differenceSignal(result.buffer, source));
    }

    std::printf("wrote %s | proposals=%zu | LUFS=%.2f target=%.2f shortfall=%.2f LU | "
                "truePeak=%.2f dBTP\n",
                options.outputPath.c_str(), result.proposals.size(), report.integratedLufs,
                report.targetLufs, report.shortfallLu, report.truePeakDb);
    return 0;
}
