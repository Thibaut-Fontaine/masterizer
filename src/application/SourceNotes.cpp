#include "masterizer/application/SourceNotes.h"

#include <cstddef>
#include <string>
#include <vector>

namespace masterizer::application {
namespace {

void appendSources(std::string& text, const std::vector<std::string>& sources) {
    if (sources.empty()) {
        return;
    }
    text += " Possible sources: ";
    for (std::size_t i = 0; i < sources.size(); ++i) {
        if (i != 0) {
            text += ", ";
        }
        text += sources[i];
    }
}

}  // namespace

std::string sourceNoteText(const domain::matching::MixSuggestion& suggestion) {
    std::string text = suggestion.id + " [" + suggestion.confidence + "] " + suggestion.action;
    appendSources(text, suggestion.plausibleSources);
    text += " (verify by ear)";
    return text;
}

std::string sourceNotesBlock(const std::vector<domain::matching::MixSuggestion>& suggestions) {
    if (suggestions.empty()) {
        return {};
    }
    std::string block = "Source notes (fix at source; verify by ear):\n";
    for (const domain::matching::MixSuggestion& suggestion : suggestions) {
        block += "  - " + sourceNoteText(suggestion) + "\n";
    }
    return block;
}

}  // namespace masterizer::application
