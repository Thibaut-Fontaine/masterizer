#include "masterizer/domain/dsp/BpmRelease.h"

#include <algorithm>
#include <cmath>

namespace masterizer::domain::dsp {
namespace {

constexpr double kDefaultReleaseSeconds = 0.1;
constexpr double kMinReleaseSeconds = 0.02;
constexpr double kMaxReleaseSeconds = 1.0;

}  // namespace

double suggestReleaseSeconds(double bpm, double beatFraction) {
    if (!std::isfinite(bpm) || bpm <= 0.0 || beatFraction <= 0.0) {
        return kDefaultReleaseSeconds;
    }
    const double secondsPerBeat = 60.0 / bpm;
    return std::clamp(secondsPerBeat * beatFraction, kMinReleaseSeconds, kMaxReleaseSeconds);
}

}  // namespace masterizer::domain::dsp
