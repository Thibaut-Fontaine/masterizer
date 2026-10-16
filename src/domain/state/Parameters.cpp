#include "masterizer/domain/state/Parameters.h"

#include <algorithm>
#include <vector>

namespace masterizer::domain::state {

ParameterSpec specOf(ParameterId id) {
    switch (id) {
        case ParameterId::MasterEnabled:
            return {"master_enabled", 0.0, 1.0, 1.0};
        case ParameterId::MatchIntensity:
            return {"match_intensity", 0.0, 2.0, 1.0};
        case ParameterId::TruePeakCeilingDb:
            return {"true_peak_ceiling_db", -24.0, 0.0, -1.0};
        case ParameterId::OutputGainDb:
            return {"output_gain_db", -24.0, 24.0, 0.0};
        case ParameterId::EqEnabled:
            return {"eq_enabled", 0.0, 1.0, 1.0};
        case ParameterId::MonoBassEnabled:
            return {"mono_bass_enabled", 0.0, 1.0, 1.0};
        case ParameterId::SaturationEnabled:
            return {"saturation_enabled", 0.0, 1.0, 0.0};
        case ParameterId::StereoEnabled:
            return {"stereo_enabled", 0.0, 1.0, 0.0};
        case ParameterId::TransientEnabled:
            return {"transient_enabled", 0.0, 1.0, 0.0};
        case ParameterId::GlueEnabled:
            return {"glue_enabled", 0.0, 1.0, 0.0};
        case ParameterId::MultibandEnabled:
            return {"multiband_enabled", 0.0, 1.0, 0.0};
        case ParameterId::DynamicEqEnabled:
            return {"dynamic_eq_enabled", 0.0, 1.0, 0.0};
        case ParameterId::ResonanceEnabled:
            return {"resonance_enabled", 0.0, 1.0, 0.0};
    }
    return {"unknown", 0.0, 1.0, 0.0};
}

double clampParameter(ParameterId id, double value) {
    const ParameterSpec spec = specOf(id);
    return std::clamp(value, spec.minValue, spec.maxValue);
}

std::vector<ParameterId> allParameters() {
    return {ParameterId::MasterEnabled,      ParameterId::MatchIntensity,
            ParameterId::TruePeakCeilingDb,  ParameterId::OutputGainDb,
            ParameterId::EqEnabled,          ParameterId::MonoBassEnabled,
            ParameterId::SaturationEnabled,  ParameterId::StereoEnabled,
            ParameterId::TransientEnabled,   ParameterId::GlueEnabled,
            ParameterId::MultibandEnabled,   ParameterId::DynamicEqEnabled,
            ParameterId::ResonanceEnabled};
}

}  // namespace masterizer::domain::state
