#pragma once

namespace masterizer::application {

/// Delivery profile. Soft is safe for lossy encoding; Loud matches hot
/// references but leaves little codec headroom.
enum class MasterProfile {
    Soft,
    Loud,
};

/// True Peak ceiling for a profile: Soft -1.0 dBTP, Loud -0.1 dBTP.
double ceilingDbFor(MasterProfile profile);

const char* profileName(MasterProfile profile);

}  // namespace masterizer::application
