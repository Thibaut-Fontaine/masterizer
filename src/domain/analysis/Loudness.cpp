#include "masterizer/domain/analysis/Loudness.h"

#include "masterizer/domain/dsp/Biquad.h"
#include "masterizer/domain/dsp/KWeighting.h"

#include <cmath>
#include <limits>
#include <vector>

namespace masterizer::domain::analysis {
namespace {

constexpr double kLoudnessOffset = -0.691;
constexpr double kAbsoluteGateLufs = -70.0;
constexpr double kRelativeGateLu = 10.0;
constexpr double kBlockSeconds = 0.4;
constexpr double kHopSeconds = 0.1;

double toLufs(double meanSquare) {
    return kLoudnessOffset + 10.0 * std::log10(meanSquare);
}

/// Mean square of the K-weighted signal for every 400 ms block (100 ms hop).
/// Streams the signal, so memory use is independent of the signal length.
std::vector<double> blockMeanSquares(const float* samples, int count, double sampleRate) {
    std::vector<double> blocks;
    const int blockSize = static_cast<int>(std::lround(kBlockSeconds * sampleRate));
    const int hop = static_cast<int>(std::lround(kHopSeconds * sampleRate));
    if (blockSize <= 0 || hop <= 0 || count < blockSize) {
        return blocks;
    }
    const dsp::BiquadCoefficients shelf = dsp::kWeightingShelf(sampleRate);
    const dsp::BiquadCoefficients highPass = dsp::kWeightingHighPass(sampleRate);
    dsp::BiquadState shelfState;
    dsp::BiquadState highPassState;
    const std::size_t size = static_cast<std::size_t>(blockSize);
    std::vector<double> window(size, 0.0);
    double running = 0.0;
    int cursor = 0;
    for (int i = 0; i < count; ++i) {
        double value = dsp::processBiquad(shelf, shelfState, samples[i]);
        value = dsp::processBiquad(highPass, highPassState, value);
        const double squared = value * value;
        running += squared - window[static_cast<std::size_t>(cursor)];
        window[static_cast<std::size_t>(cursor)] = squared;
        cursor = (cursor + 1) % blockSize;
        if (i + 1 >= blockSize && (i + 1 - blockSize) % hop == 0) {
            blocks.push_back(running / blockSize);
        }
    }
    return blocks;
}

double gatedMeanSquare(const std::vector<double>& blocks) {
    std::vector<double> absolute;
    for (double block : blocks) {
        if (toLufs(block) >= kAbsoluteGateLufs) {
            absolute.push_back(block);
        }
    }
    if (absolute.empty()) {
        return -1.0;
    }
    double absoluteSum = 0.0;
    for (double block : absolute) {
        absoluteSum += block;
    }
    const double relativeThreshold = toLufs(absoluteSum / absolute.size()) - kRelativeGateLu;
    double gatedSum = 0.0;
    int gatedCount = 0;
    for (double block : absolute) {
        if (toLufs(block) >= relativeThreshold) {
            gatedSum += block;
            ++gatedCount;
        }
    }
    return gatedCount > 0 ? gatedSum / gatedCount : -1.0;
}

}  // namespace

double integratedLufs(const float* samples, int count, double sampleRate) {
    const double negInfinity = -std::numeric_limits<double>::infinity();
    if (samples == nullptr || count <= 0 || sampleRate <= 0.0) {
        return negInfinity;
    }
    const std::vector<double> blocks = blockMeanSquares(samples, count, sampleRate);
    if (blocks.empty()) {
        return negInfinity;
    }
    const double meanSquare = gatedMeanSquare(blocks);
    return meanSquare > 0.0 ? toLufs(meanSquare) : negInfinity;
}

}  // namespace masterizer::domain::analysis
