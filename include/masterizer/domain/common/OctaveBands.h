#pragma once

namespace masterizer::domain {

/// The ten octave-band regions used as the primary analysis/UI representation.
/// Sub and VeryHigh are edge bands and are open at one end.
enum class OctaveBand {
    Sub,
    Low,
    UpperLow,
    LowMid,
    Mid,
    UpperMid,
    HighMid,
    Presence,
    High,
    VeryHigh,
    Count,
};

/// Number of octave bands (excludes the Count sentinel).
int bandCount();

/// Lower edge in Hz. Sub starts at 0 Hz, i.e. open below.
double lowerEdgeHz(OctaveBand band);

/// Upper edge in Hz. VeryHigh is open above and returns infinity.
double upperEdgeHz(OctaveBand band);

/// Geometric centre in Hz, or 0 for edge bands where it is undefined.
double centreHz(OctaveBand band);

}  // namespace masterizer::domain
