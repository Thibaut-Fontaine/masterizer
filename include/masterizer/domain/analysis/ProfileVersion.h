#pragma once

namespace masterizer::domain::analysis {

/// Version stamp of the analysis algorithm, used to invalidate cached profiles.
struct ProfileVersion {
    int major = 1;
    int minor = 0;
};

/// Version produced by the current analysis code.
inline constexpr ProfileVersion kCurrentProfileVersion{2, 0};

/// Compatible when the major versions match (minor differences are additive).
bool isCompatible(const ProfileVersion& a, const ProfileVersion& b);

}  // namespace masterizer::domain::analysis
