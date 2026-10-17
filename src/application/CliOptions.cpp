#include "masterizer/application/CliOptions.h"

#include <cstdlib>
#include <string>
#include <cstddef>
#include <vector>

namespace masterizer::application {
namespace {

struct ModuleFlag {
    const char* flag;
    const char* module;
    bool value;
};

const ModuleFlag kModuleFlags[] = {
    {"--eq", "eq", true},               {"--no-eq", "eq", false},
    {"--mono", "mono", true},           {"--no-mono", "mono", false},
    {"--limiter", "limiter", true},     {"--no-limiter", "limiter", false},
    {"--saturation", "saturation", true}, {"--no-saturation", "saturation", false},
    {"--tape", "tape", true},           {"--no-tape", "tape", false},
    {"--resonance", "resonance", true}, {"--no-resonance", "resonance", false},
    {"--stereo", "stereo", true},       {"--no-stereo", "stereo", false},
    {"--transient", "transient", true}, {"--no-transient", "transient", false},
    {"--glue", "glue", true},           {"--no-glue", "glue", false},
    {"--multiband", "multiband", true}, {"--no-multiband", "multiband", false},
    {"--dynamic-eq", "dynamic-eq", true}, {"--no-dynamic-eq", "dynamic-eq", false},
};

bool applyModuleFlag(const std::string& flag, CliOptions& out) {
    for (const ModuleFlag& entry : kModuleFlags) {
        if (flag == entry.flag) {
            out.moduleOverrides[entry.module] = entry.value;
            return true;
        }
    }
    return false;
}

bool parseNumber(const std::string& text, double& out) {
    char* end = nullptr;
    const double value = std::strtod(text.c_str(), &end);
    if (end == text.c_str() || *end != '\0') {
        return false;
    }
    out = value;
    return true;
}

}  // namespace

ParseStatus parseCliOptions(const std::vector<std::string>& args, CliOptions& out) {
    std::vector<std::string> positional;
    for (std::size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "--help" || arg == "-h") {
            return ParseStatus::Help;
        }
        if (arg == "--ab") {
            out.writeAb = true;
        } else if (arg == "--delta") {
            out.writeDelta = true;
        } else if (arg == "--intensity" || arg == "--ceiling" || arg == "--gain") {
            if (i + 1 >= args.size() || !parseNumber(args[i + 1], out.outputGainDb)) {
                return ParseStatus::Error;
            }
            const double value = out.outputGainDb;
            if (arg == "--intensity") {
                out.matchIntensity = value;
            } else if (arg == "--ceiling") {
                out.ceilingDb = value;
            }
            ++i;
        } else if (!applyModuleFlag(arg, out)) {
            if (!arg.empty() && arg[0] == '-') {
                return ParseStatus::Error;
            }
            positional.push_back(arg);
        }
    }
    if (positional.size() < 3u) {
        return ParseStatus::Error;
    }
    out.referencePath = positional[0];
    out.sourcePath = positional[1];
    out.outputPath = positional[2];
    return ParseStatus::Ok;
}

}  // namespace masterizer::application
