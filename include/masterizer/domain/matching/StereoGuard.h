#pragma once

namespace masterizer::domain::matching {

/// Width matching is only safe when the reference is not near-mono. Below
/// `minimumWidth` the per-band width ratios are dominated by noise.
bool stereoMatchAllowed(double referenceWidth, double minimumWidth = 0.05);

}  // namespace masterizer::domain::matching
