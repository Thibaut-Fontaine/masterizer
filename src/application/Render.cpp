#include "masterizer/application/Render.h"

#include "masterizer/application/MonitoringController.h"
#include "masterizer/application/OutputValidator.h"
#include "masterizer/domain/dsp/GainStage.h"
#include "masterizer/io/WavFile.h"

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

namespace masterizer::application {
namespace {

std::string auxPath(const std::string& output, const char* tag) {
    const std::size_t dot = output.find_last_of('.');
    if (dot == std::string::npos) {
        return output + tag;
    }
    return output.substr(0, dot) + tag + output.substr(dot);
}

}  // namespace

std::string masterOutputPath(const std::string& base, MasterProfile profile) {
    const std::string tag = profile == MasterProfile::Loud ? ".loud" : ".soft";
    const std::size_t dot = base.find_last_of('.');
    if (dot == std::string::npos) {
        return base + tag;
    }
    return base.substr(0, dot) + tag + base.substr(dot);
}

bool renderMaster(const RenderJob& job, MasterProfile profile, const std::string& outputPath) {
    domain::audio::AudioBuffer buffer = job.processed.buffer;
    const double ceiling = job.options.ceilingExplicit
                               ? job.options.ceilingDb
                               : ceilingDbFor(profile);
    const LoudnessTargetResult targeted = targetLoudness(
        buffer, job.reference.loudness.integratedLufs, ceiling, 12.0, job.options.outputGainDb,
        job.limiterEnabled, job.options.crestGuard, job.reference.envelope.crestDb, 0.5);

    if (!io::writeWav(outputPath, buffer)) {
        std::fprintf(stderr, "error: could not write '%s'\n", outputPath.c_str());
        return false;
    }
    if (job.options.writeAb) {
        domain::audio::AudioBuffer ab = job.sourceAudio;
        const double gainDb = loudnessMatchGainDb(job.source.loudness.integratedLufs,
                                                  targeted.achievedLufs);
        for (std::vector<float>& channel : ab.channels) {
            domain::dsp::applyGain(channel.data(), static_cast<int>(channel.size()), gainDb);
        }
        io::writeWav(auxPath(outputPath, ".ab"), ab);
    }
    if (job.options.writeDelta) {
        io::writeWav(auxPath(outputPath, ".delta"),
                     differenceSignal(buffer, job.sourceAudio));
    }

    const OutputReport report = [&] {
        OutputReport value;
        value.integratedLufs = targeted.achievedLufs;
        value.truePeakDb = targeted.truePeakDb;
        value.targetLufs = job.reference.loudness.integratedLufs;
        value.shortfallLu = targeted.shortfallLu;
        value.ceilingExceeded = targeted.truePeakDb > ceiling;
        return value;
    }();

    std::printf("wrote %s [%s] | proposals=%zu | gain=%+.2f dB | LUFS=%.2f target=%.2f "
                "shortfall=%.2f LU | truePeak=%.2f dBTP\n",
                outputPath.c_str(), profileName(profile), job.processed.proposals.size(),
                targeted.appliedGainDb, report.integratedLufs, report.targetLufs,
                report.shortfallLu, report.truePeakDb);
    return true;
}

}  // namespace masterizer::application
