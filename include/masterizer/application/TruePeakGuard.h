#pragma once

#include "masterizer/domain/audio/AudioBuffer.h"

namespace masterizer::application {

/// Scales the whole render down with a single static gain so its True Peak is
/// at or below `ceilingDb`. Never boosts. Returns the applied gain in dB (<= 0).
double enforceTruePeak(domain::audio::AudioBuffer& buffer, double ceilingDb);

}  // namespace masterizer::application
