#include "masterizer/application/OutputValidator.h"
#include "masterizer/application/ProcessingChain.h"
#include "masterizer/application/TruePeakGuard.h"
#include "masterizer/domain/analysis/SourceAnalyzer.h"
#include "masterizer/domain/audio/AudioBuffer.h"
#include "masterizer/domain/matching/DynamicsPlanner.h"
#include "masterizer/io/WavFile.h"

#include <cstdio>
#include <cstdlib>
#include <string>

namespace {

void printUsage() {
    std::fprintf(stderr,
                 "usage: masterizer_cli <reference.wav> <source.wav> <output.wav> "
                 "[intensity 0..2]\n");
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 4) {
        printUsage();
        return 2;
    }
    const std::string referencePath = argv[1];
    const std::string sourcePath = argv[2];
    const std::string outputPath = argv[3];
    const double intensity = argc > 4 ? std::atof(argv[4]) : 1.0;

    namespace app = masterizer::application;
    namespace ana = masterizer::domain::analysis;
    namespace mat = masterizer::domain::matching;

    masterizer::domain::audio::AudioBuffer reference;
    masterizer::domain::audio::AudioBuffer source;
    if (!masterizer::io::readWav(referencePath, reference)) {
        std::fprintf(stderr, "error: could not read reference '%s'\n", referencePath.c_str());
        return 1;
    }
    if (!masterizer::io::readWav(sourcePath, source)) {
        std::fprintf(stderr, "error: could not read source '%s'\n", sourcePath.c_str());
        return 1;
    }

    const ana::AnalysisSettings settings;
    const ana::AnalysisProfile referenceProfile = ana::analyzeSource(reference, settings);
    const ana::AnalysisProfile sourceProfile = ana::analyzeSource(source, settings);

    app::ChainSettings chain;
    chain.matchIntensity = intensity;
    chain.resonanceEnabled = true;
    const mat::DynamicsPlan dynamics = mat::planDynamics(sourceProfile, referenceProfile, 0.0);
    chain.glueEnabled = dynamics.glueEnabled;
    chain.glue = dynamics.glue;
    chain.multibandEnabled = dynamics.multibandEnabled;

    app::ChainResult result = app::process(source, sourceProfile, referenceProfile, chain);
    app::enforceTruePeak(result.buffer, chain.limiter.ceilingDb);
    const app::OutputReport report = app::validateOutput(
        result.buffer, referenceProfile.loudness.integratedLufs, chain.limiter.ceilingDb);

    if (!masterizer::io::writeWav(outputPath, result.buffer)) {
        std::fprintf(stderr, "error: could not write '%s'\n", outputPath.c_str());
        return 1;
    }
    std::printf("wrote %s | proposals=%zu | LUFS=%.2f target=%.2f shortfall=%.2f LU | "
                "truePeak=%.2f dBTP\n",
                outputPath.c_str(), result.proposals.size(), report.integratedLufs,
                report.targetLufs, report.shortfallLu, report.truePeakDb);
    return 0;
}
