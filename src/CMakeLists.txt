add_library(masterizer_core STATIC
    domain/analysis/Loudness.cpp
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
    domain/dsp/TruePeak.cpp)

target_include_directories(masterizer_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include)

target_compile_features(masterizer_core PUBLIC cxx_std_20)
