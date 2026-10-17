#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"

namespace masterizer::application {

struct LoudnessTargetResult {
    double appliedGainDb = 0.0;
    double achievedLufs = -70.0;
    double truePeakDb = -144.0;
    double shortfallLu = 0.0;
};

/// Raises the buffer's gain toward `targetLufs` (never more than `maxGainDb`,
/// plus `extraGainDb`), then limits to `ceilingDb` when `limit` is true, and
/// always enforces the True Peak ceiling. Reports the gain and any shortfall.
/// When `guardCrest` is set, stop before the global crest falls more than
/// `maxCrestDropDb` below `referenceCrestDb`.
LoudnessTargetResult targetLoudness(domain::audio::AudioBuffer& buffer, double targetLufs,
                                    double ceilingDb, double maxGainDb, double extraGainDb,
                                    bool limit, bool guardCrest = false,
                                    double referenceCrestDb = 0.0,
                                    double maxCrestDropDb = 0.5);

}  // namespace masterizer::application
