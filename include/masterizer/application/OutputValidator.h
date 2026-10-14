#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"

namespace masterizer::application {

/// Result of validating the rendered output against the target and ceiling.
struct OutputReport {
    double truePeakDb = -144.0;
    double integratedLufs = -70.0;
    double targetLufs = -70.0;
    double shortfallLu = 0.0;  // target - achieved, clamped at 0 (>=0 means short)
    bool ceilingExceeded = false;
};

/// Measures the render and reports loudness/True Peak against target and ceiling.
OutputReport validateOutput(const domain::audio::AudioBuffer& buffer, double targetLufs,
                            double ceilingDb);

}  // namespace masterizer::application
