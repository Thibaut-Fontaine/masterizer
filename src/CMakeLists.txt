add_library(masterizer_core STATIC
    application/OutputValidator.cpp
    application/MonitoringController.cpp
    application/ProcessingChain.cpp
    domain/analysis/BandMapping.cpp
    domain/analysis/BandFrames.cpp
    domain/analysis/BandDynamics.cpp
    domain/analysis/BandTransient.cpp
    domain/analysis/BandResonance.cpp
    domain/analysis/EnvelopeProfile.cpp
    domain/analysis/Loudness.cpp
    domain/analysis/LoudnessProfile.cpp
    domain/analysis/OctaveProfile.cpp
    domain/analysis/PowerSpectrum.cpp
    domain/analysis/ProfileVersion.cpp
    domain/analysis/SpectralFraming.cpp
    domain/analysis/SourceAnalyzer.cpp
    domain/analysis/StereoProfile.cpp
    domain/analysis/Stft.cpp
    domain/audio/AudioBufferOps.cpp
    domain/common/OctaveBands.cpp
    domain/dsp/Biquad.cpp
    domain/dsp/Correlation.cpp
    domain/dsp/Compressor.cpp
    domain/dsp/CrestFactor.cpp
    domain/dsp/Fft.cpp
    domain/dsp/Finite.cpp
    domain/dsp/Gain.cpp
    domain/dsp/KWeighting.cpp
    domain/dsp/Limiter.cpp
    domain/dsp/GainStage.cpp
    domain/dsp/MidSide.cpp
    domain/dsp/MonoBass.cpp
    domain/dsp/Peak.cpp
    domain/dsp/ParametricEq.cpp
    domain/dsp/Rms.cpp
    domain/dsp/TruePeak.cpp
    domain/matching/ProfileDifference.cpp
    domain/matching/MatchingEngine.cpp
    domain/matching/MatchIntensity.cpp
    domain/matching/MixAdvisor.cpp
    domain/matching/RuleContext.cpp
    domain/matching/RulesDynamics.cpp
    domain/matching/RulesLowEnd.cpp
    domain/matching/RulesStereo.cpp
    domain/matching/RulesTone.cpp
    domain/matching/ProfileDifference.cpp
    domain/matching/EqDesigner.cpp
    domain/state/ProfileCache.cpp
    domain/state/ReferenceKeyOps.cpp
    domain/state/ReferenceManager.cpp)

target_include_directories(masterizer_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include)

target_compile_features(masterizer_core PUBLIC cxx_std_20)
