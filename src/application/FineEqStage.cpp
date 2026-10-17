#include "masterizer/application/FineEqStage.h"

#include "masterizer/domain/dsp/ParametricEq.h"
#include "masterizer/domain/matching/FineEqDesigner.h"

#include <cstddef>
#include <vector>

namespace masterizer::application {

void applyFineEqStage(domain::audio::AudioBuffer& buffer,
                      const domain::analysis::AnalysisProfile& source,
                      const domain::analysis::AnalysisProfile& reference,
                      const domain::matching::CorrectionBounds& bounds) {
    const int fftSize =
        source.spectrum.empty() ? 0 : 2 * (static_cast<int>(source.spectrum.size()) - 1);
    if (fftSize <= 0) {
        return;
    }
    const std::vector<domain::dsp::EqBand> bands = domain::matching::designFineEq(
        source.spectrum, reference.spectrum, buffer.sampleRate, fftSize, bounds);
    for (std::vector<float>& channel : buffer.channels) {
        domain::dsp::applyEqBands(channel.data(), static_cast<int>(channel.size()),
                                  buffer.sampleRate, bands);
    }
}

}  // namespace masterizer::application
