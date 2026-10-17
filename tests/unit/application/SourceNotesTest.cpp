#include "masterizer/application/SourceNotes.h"

#include "support/Checks.h"
#include "support/TestRegistry.h"

#include <string>
#include <vector>

using masterizer::application::sourceNoteText;
using masterizer::application::sourceNotesBlock;
using masterizer::domain::matching::MixSuggestion;

namespace {

MixSuggestion kickSuggestion() {
    MixSuggestion suggestion;
    suggestion.id = "R1";
    suggestion.action = "Try +1-2 dB on the kick, or carve the bass under it.";
    suggestion.confidence = "high";
    suggestion.plausibleSources = {"kick (likely)", "bass masking (possible)"};
    return suggestion;
}

}  // namespace

TEST_CASE(a_source_note_is_hedged_and_actionable) {
    const std::string text = sourceNoteText(kickSuggestion());
    CHECK(text.find("R1") != std::string::npos);
    CHECK(text.find("+1-2 dB") != std::string::npos);
    CHECK(text.find("high") != std::string::npos);
    CHECK(text.find("kick (likely)") != std::string::npos);
    CHECK(text.find("verify by ear") != std::string::npos);
}

TEST_CASE(a_source_notes_block_is_empty_when_there_is_nothing_to_say) {
    CHECK(sourceNotesBlock({}).empty());
}

TEST_CASE(a_source_notes_block_lists_every_suggestion) {
    MixSuggestion second = kickSuggestion();
    second.id = "R5";
    second.action = "Lift air/presence at source.";
    const std::string block = sourceNotesBlock({kickSuggestion(), second});
    CHECK(block.find("Source notes") != std::string::npos);
    CHECK(block.find("R1") != std::string::npos);
    CHECK(block.find("R5") != std::string::npos);
    CHECK(block.find("verify by ear") != std::string::npos);
}
