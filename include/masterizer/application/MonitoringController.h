#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"

namespace masterizer::application {

/// Monitoring gain (dB) to apply to the processed signal so it is heard at the
/// same loudness as the original. Monitoring only; never affects the exported
/// level. Returns 0 for non-finite inputs.
double loudnessMatchGainDb(double originalLufs, double processedLufs);

/// Delta signal = processed - original, sample-aligned and length-matched.
domain::audio::AudioBuffer differenceSignal(const domain::audio::AudioBuffer& processed,
                                            const domain::audio::AudioBuffer& original);

}  // namespace masterizer::application
