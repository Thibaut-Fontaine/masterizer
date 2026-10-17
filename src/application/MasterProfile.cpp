#include "masterizer/application/MasterProfile.h"

namespace masterizer::application {

double ceilingDbFor(MasterProfile profile) {
    return profile == MasterProfile::Loud ? -0.1 : -1.0;
}

const char* profileName(MasterProfile profile) {
    return profile == MasterProfile::Loud ? "loud" : "soft";
}

}  // namespace masterizer::application
