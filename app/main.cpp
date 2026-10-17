#include "masterizer/application/CliOptions.h"
#include "masterizer/application/LoudnessTargeter.h"
#include "masterizer/application/MonitoringController.h"
#include "masterizer/application/OutputValidator.h"
#include "masterizer/application/ProcessingChain.h"
#include "masterizer/domain/analysis/Loudness.h"
#include "masterizer/domain/analysis/SourceAnalyzer.h"
#include "masterizer/domain/audio/AudioBuffer.h"
#include "masterizer/domain/audio/AudioBufferOps.h"
#include "masterizer/domain/dsp/GainStage.h"
#include "masterizer/domain/matching/DynamicsPlanner.h"
#include "masterizer/domain/matching/StereoGuard.h"
#include "masterizer/io/AudioFile.h"
#include "masterizer/io/WavFile.h"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <future>
#include <string>
#include <vector>

namespace {

void printUsage() {
    std::fprintf(stderr,
                 "usage: masterizer_cli <reference> <source> <output.wav>\n"
                 "  inputs: WAV, FLAC or MP3 (any mix)\n"
                 "  --intensity N     Match Intensity 0..2 (default 1)\n"
                 "  --ceiling dB      True Peak ceiling (default -1)\n"
                 "  --gain dB         output gain (default 0)\n"
                 "  --ab              also write a level-matched <output>.ab.wav\n"
                 "  --delta           also write <output>.delta.wav (processed - source)\n"
                 "  --timing          print per-phase timings\n"
                 "  Loudness is targeted toward the reference (up to +12 dB), then limited.\n"
                 "  --<module> / --no-<module>\n"
                 "      module: eq mono limiter saturation tape resonance stereo transient\n"
                 "              glue multiband dynamic-eq\n");
}

void applyOverrides(masterizer::application::ChainSettings& chain,
                    const masterizer::application::CliOptions& options) {
    for (const auto& [name, value] : options.moduleOverrides) {
        if (name == "eq") chain.eqEnabled = value;
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

std::string auxPath(const std::string& output, const char* tag) {
    const std::size_t dot = output.find_last_of('.');
    if (dot == std::string::npos) {
        return output + tag;
    }
    return output.substr(0, dot) + tag + output.substr(dot);
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

double lufsOf(const masterizer::domain::audio::AudioBuffer& buffer) {
    namespace ana = masterizer::domain::analysis;
    const int frames = masterizer::domain::audio::frameCount(buffer);
    if (frames <= 0) {
        return -70.0;
    }
    if (buffer.channels.size() >= 2) {
        return ana::integratedLufs(buffer.channels[0].data(), buffer.channels[1].data(),
                                   frames, buffer.sampleRate);
    }
    return ana::integratedLufs(buffer.channels[0].data(), frames, buffer.sampleRate);
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

    app::ChainSettings chain;
    chain.matchIntensity = options.matchIntensity;
    chain.outputGainDb = options.outputGainDb;
    chain.limiter.ceilingDb = options.ceilingDb;
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

    app::ChainResult result = app::process(source, sourceProfile, referenceProfile, chain);
    const Clock::time_point tProcess = Clock::now();

    const app::LoudnessTargetResult targeted = app::targetLoudness(
        result.buffer, referenceProfile.loudness.integratedLufs, chain.limiter.ceilingDb,
        12.0, options.outputGainDb, limiterEnabled, true,
        referenceProfile.envelope.crestDb, 0.5);
    const Clock::time_point tTarget = Clock::now();

    // Reuse the targeting result instead of re-measuring loudness and peak.
    app::OutputReport report;
    report.integratedLufs = targeted.achievedLufs;
    report.truePeakDb = targeted.truePeakDb;
    report.targetLufs = referenceProfile.loudness.integratedLufs;
    report.shortfallLu = targeted.shortfallLu;
    report.ceilingExceeded = targeted.truePeakDb > chain.limiter.ceilingDb;
    const Clock::time_point tValidate = Clock::now();

    if (!masterizer::io::writeWav(options.outputPath, result.buffer)) {
        std::fprintf(stderr, "error: could not write '%s'\n", options.outputPath.c_str());
        return 1;
    }
    if (options.writeAb) {
        masterizer::domain::audio::AudioBuffer ab = source;
        const double sourceLufs = lufsOf(source);
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
    const Clock::time_point tWrite = Clock::now();

    std::printf("wrote %s | proposals=%zu | gain=%+.2f dB | LUFS=%.2f target=%.2f "
                "shortfall=%.2f LU | truePeak=%.2f dBTP\n",
                options.outputPath.c_str(), result.proposals.size(), targeted.appliedGainDb,
                report.integratedLufs, report.targetLufs, report.shortfallLu,
                report.truePeakDb);

    if (options.timing) {
        const auto ms = [](Clock::time_point a, Clock::time_point b) {
            return std::chrono::duration<double, std::milli>(b - a).count();
        };
        std::printf("timing(ms): read=%.0f analyze=%.0f process=%.0f target=%.0f "
                    "validate=%.0f write=%.0f total=%.0f\n",
                    ms(t0, tRead), ms(tRead, tAnalyze), ms(tAnalyze, tProcess),
                    ms(tProcess, tTarget), ms(tTarget, tValidate), ms(tValidate, tWrite),
                    ms(t0, tWrite));
    }
    return 0;
}
