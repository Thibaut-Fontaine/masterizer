#pragma once

namespace masterizer::domain::matching {

/// Which module a proposed correction targets.
enum class CorrectionTarget {
    Eq,
    Dynamics,
    Transient,
    Stereo,
    Peak,
    Loudness,
};

/// Direction of a correction relative to the Source.
enum class CorrectionDirection {
    Cut,
    Boost,
    None,
};

/// A bounded, confidence-tagged correction proposed for review before applying.
struct CorrectionProposal {
    CorrectionTarget target = CorrectionTarget::Eq;
    int band = -1;  // octave band index, or -1 for a global proposal
    CorrectionDirection direction = CorrectionDirection::None;
    double amountDb = 0.0;   // signed correction: negative = cut, positive = boost
    double confidence = 0.0;  // 0..1
    const char* reason = "";
};

}  // namespace masterizer::domain::matching
