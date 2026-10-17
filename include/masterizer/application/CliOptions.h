#pragma once

#include <map>
#include <string>
#include <vector>

namespace masterizer::application {

/// Parsed command-line options for the offline tool.
struct CliOptions {
    std::string referencePath;
    std::string sourcePath;
    std::string outputPath;
    double matchIntensity = 1.0;
    double ceilingDb = -1.0;
    double outputGainDb = 0.0;
    bool writeAb = false;
    bool writeDelta = false;
    bool timing = false;
    bool crestGuard = true;
    // Module overrides keyed by name ("eq", "mono", "glue", ...); anything absent
    // keeps the default/planner decision.
    std::map<std::string, bool> moduleOverrides;
};

enum class ParseStatus { Ok, Help, Error };

/// Parses argv-style arguments (index 0 is the program name). Three positional
/// paths are required. Returns Help for --help/-h and Error for anything invalid.
ParseStatus parseCliOptions(const std::vector<std::string>& args, CliOptions& out);

}  // namespace masterizer::application
