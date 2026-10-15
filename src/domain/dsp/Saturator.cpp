#include "masterizer/domain/dsp/Saturator.h"

#include "masterizer/domain/dsp/Gain.h"

#include <algorithm>
#include <cmath>

namespace masterizer::domain::dsp {

void applySaturation(float* samples, int count, const SaturationSettings& settings) {
    if (samples == nullptr || count <= 0 || settings.driveDb <= 0.0 || settings.mix <= 0.0) {
        return;
    }
    const double drive = dbToLinear(settings.driveDb);
    const double compensation = 1.0 / drive;  // preserves small-signal gain
    const double mix = std::clamp(settings.mix, 0.0, 1.0);
    for (int i = 0; i < count; ++i) {
        const double input = static_cast<double>(samples[i]);
        const double shaped = std::tanh(drive * input) * compensation;
        samples[i] = static_cast<float>((1.0 - mix) * input + mix * shaped);
    }
}

}  // namespace masterizer::domain::dsp
