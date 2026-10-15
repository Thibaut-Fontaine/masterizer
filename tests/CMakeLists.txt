add_executable(masterizer_tests
    support/Buffers.cpp
    support/FakeDecoder.cpp
    support/Main.cpp
    support/Profiles.cpp
    support/Signals.cpp
    support/SignalsProgram.cpp
    support/SignalsSpectral.cpp
    support/Stereo.cpp
    support/TestContext.cpp
    support/TestRegistry.cpp
    unit/analysis/BandFramesTest.cpp
    unit/analysis/BandMappingTest.cpp
    unit/analysis/BandDynamicsTest.cpp
    unit/analysis/BandTransientTest.cpp
    unit/analysis/BandResonanceTest.cpp
    unit/analysis/EnvelopeProfileTest.cpp
    unit/analysis/LoudnessTest.cpp
    unit/analysis/LoudnessProfileTest.cpp
    unit/analysis/OctaveProfileTest.cpp
    unit/analysis/ProfileVersionTest.cpp
    unit/analysis/PowerSpectrumTest.cpp
    unit/analysis/SpectralFramingTest.cpp
    unit/analysis/SourceAnalyzerTest.cpp
    unit/analysis/StereoProfileTest.cpp
    unit/analysis/StftTest.cpp
    unit/application/OutputValidatorTest.cpp
    unit/application/ProcessingChainTest.cpp
    unit/application/ProcessingChainStagesTest.cpp
    unit/application/ChainReporterTest.cpp
    unit/application/MonitoringControllerTest.cpp
    unit/audio/AudioBufferOpsTest.cpp
    unit/common/OctaveBandsTest.cpp
    unit/dsp/BiquadTest.cpp
    unit/dsp/BpmReleaseTest.cpp
    unit/dsp/CorrelationTest.cpp
    unit/dsp/CompressorTest.cpp
    unit/dsp/CrestFactorTest.cpp
    unit/dsp/DynamicEqTest.cpp
    unit/dsp/FftTest.cpp
    unit/dsp/FiniteTest.cpp
    unit/dsp/GainTest.cpp
    unit/dsp/GainStageTest.cpp
    unit/dsp/FilterDesignTest.cpp
    unit/dsp/KWeightingTest.cpp
    unit/dsp/LimiterTest.cpp
    unit/dsp/MidSideTest.cpp
    unit/dsp/MonoBassTest.cpp
    unit/dsp/MultibandCompressorTest.cpp
    unit/dsp/ParametricEqTest.cpp
    unit/dsp/PeakTest.cpp
    unit/dsp/RmsTest.cpp
    unit/dsp/SaturatorTest.cpp
    unit/dsp/StereoWidthTest.cpp
    unit/dsp/TruePeakTest.cpp
    unit/dsp/TransientShaperTest.cpp
    unit/matching/EqDesignerTest.cpp
    unit/matching/ChainCoordinatorTest.cpp
    unit/matching/DynamicsPlannerTest.cpp
    unit/matching/MatchingEngineTest.cpp
    unit/matching/MatchIntensityTest.cpp
    unit/matching/MixAdvisorTest.cpp
    unit/matching/ProfileDifferenceTest.cpp
    unit/matching/ResonanceDetectorTest.cpp
    unit/support/SignalsProgramTest.cpp
    unit/support/SignalsSpectralTest.cpp
    unit/support/SignalsTest.cpp
    unit/support/StereoTest.cpp
    unit/state/ProfileCacheTest.cpp
    unit/state/ReferenceKeyOpsTest.cpp
    unit/state/ReferenceManagerTest.cpp)

target_link_libraries(masterizer_tests PRIVATE masterizer_core)
target_include_directories(masterizer_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME unit COMMAND masterizer_tests)

find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
    add_test(NAME style_rules
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/check_rules.py
                ${CMAKE_SOURCE_DIR})
endif()
