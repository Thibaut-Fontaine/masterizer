#include "masterizer/domain/state/ProfileCache.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <memory>

using masterizer::domain::analysis::AnalysisProfile;
using masterizer::domain::analysis::ProfileVersion;
using masterizer::domain::state::ProfileCache;
using masterizer::domain::state::ReferenceKey;

namespace {

const ReferenceKey kReference{"ref.wav", 2048u, 7};

}  // namespace

TEST_CASE(store_then_lookup_returns_the_profile) {
    ProfileCache cache;
    auto profile = std::make_shared<AnalysisProfile>();
    profile->loudness.integratedLufs = -9.5;
    cache.store(kReference, profile);
    const auto found = cache.lookup(kReference);
    CHECK(found != nullptr);
    CHECK_NEAR(found->loudness.integratedLufs, -9.5, 1e-12);
    CHECK(cache.size() == 1);
}

TEST_CASE(lookup_of_unknown_key_is_null) {
    ProfileCache cache;
    CHECK(cache.lookup(kReference) == nullptr);
}

TEST_CASE(incompatible_profile_version_is_not_returned) {
    ProfileCache cache;
    auto profile = std::make_shared<AnalysisProfile>();
    profile->version = ProfileVersion{2, 0};
    cache.store(kReference, profile);
    CHECK(cache.lookup(kReference) == nullptr);
}

TEST_CASE(clear_empties_the_cache) {
    ProfileCache cache;
    cache.store(kReference, std::make_shared<AnalysisProfile>());
    cache.clear();
    CHECK(cache.size() == 0);
}
