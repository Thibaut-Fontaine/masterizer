#include "masterizer/domain/dsp/GainStage.h"

#include "masterizer/domain/dsp/Gain.h"

namespace masterizer::domain::dsp {

void applyGain(float* samples, int count, double gainDb) {
    if (samples == nullptr || count <= 0) {
        return;
    }
    const double gain = dbToLinear(gainDb);
    for (int i = 0; i < count; ++i) {
        samples[i] = static_cast<float>(static_cast<double>(samples[i]) * gain);
    }
}

}  // namespace masterizer::domain::dsp
