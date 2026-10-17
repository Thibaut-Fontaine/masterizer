#pragma once

#include <string>
#include <vector>

namespace masterizer::domain::state {

/// Stable parameter identities (strings are reused as the host parameter ids).
enum class ParameterId {
    MasterEnabled,
    MasterProfile,
    MatchIntensity,
    TruePeakCeilingDb,
    OutputGainDb,
    LimiterEnabled,
    EqEnabled,
    FineEqEnabled,
    MonoBassEnabled,
    SaturationEnabled,
    TapeEnabled,
    StereoEnabled,
    TransientEnabled,
    GlueEnabled,
    MultibandEnabled,
    DynamicEqEnabled,
    ResonanceEnabled,
    CrestGuardEnabled,
};

/// Range and default for one parameter.
struct ParameterSpec {
    std::string id;
    double minValue;
    double maxValue;
    double defaultValue;
};

ParameterSpec specOf(ParameterId id);

/// Clamps a value into the parameter's range.
double clampParameter(ParameterId id, double value);

/// All parameters, in a stable order.
std::vector<ParameterId> allParameters();

}  // namespace masterizer::domain::state
