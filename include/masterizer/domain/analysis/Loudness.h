#pragma once

namespace masterizer::domain::analysis {

/// BS.1770-4 gated integrated loudness in LUFS for a single channel.
/// Uses 400 ms blocks with 100 ms hop, absolute (-70 LUFS) and relative (-10 LU)
/// gating, and the standard -0.691 dB offset. Returns -infinity when no block
/// passes the absolute gate (for example digital silence).
double integratedLufs(const float* samples, int count, double sampleRate);

}  // namespace masterizer::domain::analysis
