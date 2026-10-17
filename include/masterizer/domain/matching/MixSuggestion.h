#pragma once

#include <string>
#include <vector>

namespace masterizer::domain::matching {

/// Category of a source-level suggestion.
enum class SuggestionCategory {
    Tonal,
    Dynamics,
    Transient,
    Stereo,
    Resonance,
    Peak,
};

/// A small, honest, source-level suggestion. Never names an instrument as fact:
/// `plausibleSources` is ranked and always paired with a confidence and a
/// "verify by ear" expectation.
struct MixSuggestion {
    std::string id;
    SuggestionCategory category = SuggestionCategory::Tonal;
    std::vector<int> bands;
    std::string feature;
    std::string action;
    std::vector<std::string> plausibleSources;
    std::string confidence;  // "low" | "medium" | "high"
    bool masterFixable = true;
    std::string reasonCode;
    int rulesetVersion = 0;  // stamped by advise() so advice is traceable
};

}  // namespace masterizer::domain::matching
