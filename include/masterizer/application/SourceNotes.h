#pragma once

#include "masterizer/domain/matching/MixSuggestion.h"

#include <string>
#include <vector>

namespace masterizer::application {

/// One plain-language source note. Always ends with a verify-by-ear reminder.
std::string sourceNoteText(const domain::matching::MixSuggestion& suggestion);

/// All suggestions as a printable block. Empty when there is nothing to say.
std::string sourceNotesBlock(const std::vector<domain::matching::MixSuggestion>& suggestions);

}  // namespace masterizer::application
