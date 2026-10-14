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
    profile.octaveEnergy = octaveBandEnergies(mono.data(), frames, buffer.sampleRate,
                                              settings.fftSize, settings.hop);
    profile.spectrum = averageMagnitudeSpectrum(mono.data(), frames, settings.fftSize,
                                                settings.hop);
    profile.bandVariation = bandVariation(mono.data(), frames, buffer.sampleRate,
                                          settings.fftSize, settings.hop);
    profile.transient = analyzeBandTransient(mono.data(), frames, buffer.sampleRate,
                                             settings.fftSize, settings.hop);
    profile.resonance = analyzeBandResonance(mono.data(), frames, buffer.sampleRate,
                                             settings.fftSize, settings.hop);
    profile.envelope = analyzeEnvelope(mono.data(), frames, buffer.sampleRate);
    profile.stereo = analyzeStereo(buffer, settings.fftSize, settings.hop);
    profile.loudness = analyzeLoudness(buffer);
    return profile;
}

}  // namespace masterizer::domain::analysis
