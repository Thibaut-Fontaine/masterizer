#include "masterizer/domain/analysis/ProfileVersion.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::analysis::isCompatible;
using masterizer::domain::analysis::kCurrentProfileVersion;
using masterizer::domain::analysis::ProfileVersion;

TEST_CASE(profile_versions_with_same_major_are_compatible) {
    CHECK(isCompatible({1, 0}, {1, 5}));
    CHECK(isCompatible(kCurrentProfileVersion, kCurrentProfileVersion));
}

TEST_CASE(profile_versions_with_different_major_are_incompatible) {
    CHECK(!isCompatible({1, 0}, {2, 0}));
}
