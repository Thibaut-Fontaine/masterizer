#pragma once

namespace masterizer::domain::dsp {

/// Suggested compressor release time (seconds) for a tempo, as `beatFraction`
/// of one beat. Clamped to a sane range. Returns a neutral default (0.1 s) when
/// the BPM is invalid, so tempo sync never overrides musical constraints and can
/// always be ignored.
double suggestReleaseSeconds(double bpm, double beatFraction);

}  // namespace masterizer::domain::dsp
