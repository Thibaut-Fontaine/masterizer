add_library(masterizer_core STATIC
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
    domain/dsp/CrestFactor.cpp
    domain/dsp/Fft.cpp
    domain/dsp/Finite.cpp
    domain/dsp/Gain.cpp
    domain/dsp/KWeighting.cpp
    domain/dsp/MidSide.cpp
    domain/dsp/Peak.cpp
    domain/dsp/Rms.cpp
    domain/dsp/TruePeak.cpp
    domain/matching/ProfileDifference.cpp
    domain/matching/MatchingEngine.cpp
    domain/state/ProfileCache.cpp
    domain/state/ReferenceKeyOps.cpp
    domain/state/ReferenceManager.cpp)

target_include_directories(masterizer_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include)

target_compile_features(masterizer_core PUBLIC cxx_std_20)
