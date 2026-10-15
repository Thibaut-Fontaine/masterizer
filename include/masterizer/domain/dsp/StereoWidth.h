#pragma once

namespace masterizer::domain::dsp {

/// Scales the Side component of a stereo pair in place.
/// 1.0 = unchanged, 0.0 = mono, above 1.0 = wider.
void applyStereoWidth(float* left, float* right, int count, double widthScale);

/// Side scale that moves `currentWidth` toward `targetWidth` (widths are
/// side/(mid+side) in [0, 1]); clamped to [0, 2].
double widthScaleFor(double currentWidth, double targetWidth);

}  // namespace masterizer::domain::dsp
