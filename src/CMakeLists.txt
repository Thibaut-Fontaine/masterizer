add_library(masterizer_core STATIC
    domain/common/OctaveBands.cpp
    domain/dsp/Correlation.cpp
    domain/dsp/CrestFactor.cpp
    domain/dsp/Finite.cpp
    domain/dsp/MidSide.cpp
    domain/dsp/Peak.cpp
    domain/dsp/Rms.cpp)

target_include_directories(masterizer_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include)

target_compile_features(masterizer_core PUBLIC cxx_std_20)
