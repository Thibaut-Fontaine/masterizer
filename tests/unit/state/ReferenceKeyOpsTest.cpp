#include "masterizer/domain/state/ReferenceKeyOps.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

using masterizer::domain::state::cacheId;
using masterizer::domain::state::ReferenceKey;
using masterizer::domain::state::sameReference;

namespace {

const ReferenceKey kReference{"song.wav", 1000u, 42};

}  // namespace

TEST_CASE(same_reference_compares_all_fields) {
    CHECK(sameReference(kReference, kReference));
    CHECK(!sameReference(kReference, {"song.wav", 1001u, 42}));
    CHECK(!sameReference(kReference, {"song.wav", 1000u, 43}));
    CHECK(!sameReference(kReference, {"other.wav", 1000u, 42}));
}

TEST_CASE(cache_id_is_stable_and_distinct) {
    CHECK(cacheId(kReference) == cacheId(kReference));
    CHECK(cacheId(kReference) != cacheId({"other.wav", 1000u, 42}));
    CHECK(cacheId(kReference) != cacheId({"song.wav", 999u, 42}));
}
