#include "masterizer/application/OutputValidator.h"

#include "masterizer/domain/analysis/LoudnessProfile.h"

#include <algorithm>

namespace masterizer::application {

OutputReport validateOutput(const domain::audio::AudioBuffer& buffer, double targetLufs,
                            double ceilingDb) {
    const domain::analysis::LoudnessProfile loudness =
        domain::analysis::analyzeLoudness(buffer);
    OutputReport report;
    report.truePeakDb = loudness.truePeakDb;
    report.integratedLufs = loudness.integratedLufs;
    report.targetLufs = targetLufs;
    report.shortfallLu = std::max(0.0, targetLufs - loudness.integratedLufs);
    report.ceilingExceeded = loudness.truePeakDb > ceilingDb;
    return report;
}

}  // namespace masterizer::application
