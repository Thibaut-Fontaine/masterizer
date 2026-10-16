#include "masterizer/domain/matching/RuleContext.h"

#include <cstddef>
#include <vector>

namespace masterizer::domain::matching {

double energyGapDb(const RuleContext& context, int band) {
    if (band < 0 || static_cast<std::size_t>(band) >= context.difference.bands.size()) {
        return 0.0;
    }
    return context.difference.bands[static_cast<std::size_t>(band)].differenceDb;
}

double variationGap(const RuleContext& context, int band) {
    const std::vector<double>& source = context.source.bandVariation;
    const std::vector<double>& reference = context.reference.bandVariation;
    if (band < 0 || static_cast<std::size_t>(band) >= source.size() ||
        static_cast<std::size_t>(band) >= reference.size()) {
        return 0.0;
    }
    return source[static_cast<std::size_t>(band)] - reference[static_cast<std::size_t>(band)];
}

double densityGap(const RuleContext& context, int band) {
    const std::vector<double>& source = context.source.transient.density;
    const std::vector<double>& reference = context.reference.transient.density;
    if (band < 0 || static_cast<std::size_t>(band) >= source.size() ||
        static_cast<std::size_t>(band) >= reference.size()) {
        return 0.0;
    }
    return source[static_cast<std::size_t>(band)] - reference[static_cast<std::size_t>(band)];
}

double peakinessOf(const RuleContext& context, int band) {
    const std::vector<double>& peakiness = context.source.resonance.peakiness;
    if (band < 0 || static_cast<std::size_t>(band) >= peakiness.size()) {
        return 0.0;
    }
    return peakiness[static_cast<std::size_t>(band)];
}

double persistenceOf(const RuleContext& context, int band) {
    const std::vector<double>& persistence = context.source.resonance.persistence;
    if (band < 0 || static_cast<std::size_t>(band) >= persistence.size()) {
        return 0.0;
    }
    return persistence[static_cast<std::size_t>(band)];
}

}  // namespace masterizer::domain::matching
