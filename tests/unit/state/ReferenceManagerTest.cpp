#include "masterizer/domain/state/ReferenceManager.h"

#include "masterizer/domain/audio/DecodedAudio.h"
#include "support/Buffers.h"
#include "support/Checks.h"
#include "support/FakeDecoder.h"
#include "support/TestRegistry.h"

#include <vector>
#include <string>

using masterizer::domain::audio::DecodedAudio;
using masterizer::domain::state::ReferenceKey;
using masterizer::domain::state::ReferenceManager;
using masterizer::domain::state::ReferenceStatus;
using masterizer::test::FakeDecoder;
using masterizer::test::monoBuffer;

namespace {

const ReferenceKey kKey{"ref.wav", 500u, 11};
const ReferenceKey kChanged{"ref.wav", 501u, 12};

DecodedAudio okDecode(const std::string& path) {
    DecodedAudio decoded;
    decoded.path = path;
    decoded.succeeded = true;
    decoded.buffer = monoBuffer({0.1f, 0.2f, 0.3f, 0.4f}, 48000.0);
    return decoded;
}

}  // namespace

TEST_CASE(load_success_reports_ready_with_buffer) {
    FakeDecoder decoder;
    decoder.setResult("ref.wav", okDecode("ref.wav"));
    ReferenceManager manager(decoder);
    const auto& state = manager.load("ref.wav", kKey);
    CHECK(state.status == ReferenceStatus::Ready);
    CHECK(state.error.empty());
    CHECK(state.buffer.channels.size() == 1u);
    CHECK(state.buffer.channels[0].size() == 4u);
}

TEST_CASE(load_failure_reports_failed_with_error) {
    FakeDecoder decoder;
    ReferenceManager manager(decoder);
    const auto& missing = manager.load("missing.wav", kKey);
    CHECK(missing.status == ReferenceStatus::Failed);
    CHECK(!missing.error.empty());

    DecodedAudio empty;
    empty.path = "empty.wav";
    empty.succeeded = true;
    decoder.setResult("empty.wav", empty);
    const auto& invalid = manager.load("empty.wav", kKey);
    CHECK(invalid.status == ReferenceStatus::Failed);
}

TEST_CASE(is_stale_detects_a_changed_file) {
    FakeDecoder decoder;
    decoder.setResult("ref.wav", okDecode("ref.wav"));
    ReferenceManager manager(decoder);
    manager.load("ref.wav", kKey);
    CHECK(!manager.isStale(kKey));
    CHECK(manager.isStale(kChanged));
}

TEST_CASE(unload_resets_to_unloaded) {
    FakeDecoder decoder;
    decoder.setResult("ref.wav", okDecode("ref.wav"));
    ReferenceManager manager(decoder);
    manager.load("ref.wav", kKey);
    const auto& state = manager.unload();
    CHECK(state.status == ReferenceStatus::Unloaded);
    CHECK(state.buffer.channels.empty());
}
