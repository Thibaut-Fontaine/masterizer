#include "masterizer/domain/analysis/SourceAnalyzer.h"

#include "masterizer/domain/analysis/BandDynamics.h"
#include "masterizer/domain/analysis/BandResonance.h"
#include "masterizer/domain/analysis/BandTransient.h"
#include "masterizer/domain/analysis/EnvelopeProfile.h"
#include "masterizer/domain/analysis/LoudnessProfile.h"
#include "masterizer/domain/analysis/OctaveProfile.h"
#include "masterizer/domain/analysis/StereoProfile.h"
#include "masterizer/domain/analysis/Stft.h"
#include "masterizer/domain/audio/AudioBufferOps.h"

#include <cstddef>
#include <future>
#include <vector>

namespace masterizer::domain::analysis {
namespace {

std::vector<float> monoDownmix(const audio::AudioBuffer& buffer) {
    const int channels = audio::channelCount(buffer);
    const int frames = audio::frameCount(buffer);
    std::vector<float> mono(static_cast<std::size_t>(frames > 0 ? frames : 0), 0.0f);
    if (channels <= 0 || frames <= 0) {
        return mono;
    }
    for (int i = 0; i < frames; ++i) {
        double sum = 0.0;
        for (int channel = 0; channel < channels; ++channel) {
            sum += buffer.channels[static_cast<std::size_t>(channel)][static_cast<std::size_t>(i)];
        }
        mono[static_cast<std::size_t>(i)] = static_cast<float>(sum / channels);
    }
    return mono;
}

}  // namespace

AnalysisProfile analyzeSource(const audio::AudioBuffer& buffer,
                              const AnalysisSettings& settings) {
    AnalysisProfile profile;
    const int frames = audio::frameCount(buffer);
    if (audio::channelCount(buffer) <= 0 || frames <= 0) {
        return profile;
    }
    const std::vector<float> mono = monoDownmix(buffer);
    const float* samples = mono.data();
    const double rate = buffer.sampleRate;
    const int fftSize = settings.fftSize;
    const int hop = settings.hop;

    // Independent feature extractions, run concurrently. Each reads only the
    // shared input and writes its own result, so this is deterministic.
    auto octaveFuture = std::async(std::launch::async, [&] {
        return octaveBandEnergies(samples, frames, rate, fftSize, hop);
    });
    auto spectrumFuture = std::async(std::launch::async, [&] {
        return averageMagnitudeSpectrum(samples, frames, fftSize, hop);
    });
    auto variationFuture = std::async(std::launch::async, [&] {
        return bandVariation(samples, frames, rate, fftSize, hop);
    });
    auto transientFuture = std::async(std::launch::async, [&] {
        return analyzeBandTransient(samples, frames, rate, fftSize, hop);
    });
    auto resonanceFuture = std::async(std::launch::async, [&] {
        return analyzeBandResonance(samples, frames, rate, fftSize, hop);
    });
    auto stereoFuture = std::async(std::launch::async, [&] {
        return analyzeStereo(buffer, fftSize, hop);
    });

    profile.octaveEnergy = octaveFuture.get();
    profile.spectrum = spectrumFuture.get();
    profile.bandVariation = variationFuture.get();
    profile.transient = transientFuture.get();
    profile.resonance = resonanceFuture.get();
    profile.stereo = stereoFuture.get();
    profile.envelope = analyzeEnvelope(samples, frames, rate);
    profile.loudness = analyzeLoudness(buffer);
    return profile;
}

}  // namespace masterizer::domain::analysis
