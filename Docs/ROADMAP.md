# Development Roadmap — Reference-Based Automatic Mastering VST

> Required implementation order (formerly the root `STEPS.md`). Build one
> buildable step at a time; see `/AGENTS.md` for working rules.

I recommend building the plugin with C++20, JUCE, CMake and VST3, starting with a working mastering prototype before implementing the advanced DSP modules.
The most important rule is to build and validate each layer independently. Do not implement all nine processing modules at once.
Phase 1 — Project setup
Step 1. Define the development environment
Establish the initial development targets:
- Language: C++20.
- Audio framework: JUCE.
- Build system: CMake.
- Initial plugin format: VST3.
- Initial platform: Linux, with Windows and macOS compatibility as subsequent targets if cross-platform distribution is required.
- Testing: Automated DSP tests, plugin validation and testing in a real DAW.
- Version control: Git.
Use a pinned, tested JUCE release and avoid automatically upgrading dependencies.
Before committing to JUCE, review its current licensing conditions. Its licensing options have different commercial terms and revenue limits. Steinberg's VST3 SDK has been available under the MIT licence since version 3.8, but this does not replace the separate licensing requirements of JUCE. (JUCE licensing, Steinberg VST3 licensing)
Deliverable: A documented development environment and a clear choice of dependencies.
Step 2. Create the repository and build system
Create the Git repository and configure the CMake project.
Establish a structure along these lines:
reference-mastering/
├── CMakeLists.txt
├── CMakePresets.json
├── AGENTS.md
├── README.md
├── Source/
│   ├── PluginProcessor.h
│   ├── PluginProcessor.cpp
│   ├── PluginEditor.h
│   ├── PluginEditor.cpp
│   ├── Analysis/
│   ├── Matching/
│   ├── DSP/
│   └── State/
├── Tests/
│   ├── AnalysisTests.cpp
│   ├── MatchingTests.cpp
│   └── DSPTests.cpp
├── Scripts/
└── docs/

The precise file structure can evolve as the implementation grows, but the separation between analysis, matching and DSP should remain.
Configure Debug and Release builds.
Deliverable: A repository that configures and builds successfully.
Step 3. Build a minimal working VST3
Create the smallest possible plugin.
It should:
- Load in a DAW.
- Receive stereo audio.
- Pass audio through unchanged.
- Support bypass.
- Expose a basic user interface.
- Restore its state when a DAW project is reopened.
Do not implement reference analysis or mastering algorithms yet.
Test the plugin in your target DAW and confirm that it behaves correctly during playback, seeking, project reopening and offline rendering.
Deliverable: A stable, working VST3 skeleton.
This is the first major milestone. Until it works reliably, adding DSP will only make debugging more difficult.
Phase 2 — Audio infrastructure
Step 4. Implement parameter and state management
Create the parameter system before adding the processing algorithms.
Define parameters for:
- Mastering enable/bypass.
- Match Intensity.
- Reference selection and analysis status.
- Module enable/disable states.
- Processing controls.
- Output gain.
- True Peak ceiling.
Parameters must remain valid when the DAW automates them or restores a saved project.
Reference metadata and analysis profiles should also be preserved where appropriate. If a referenced file is missing when a project is reopened, the plugin must report the problem without crashing.
Avoid creating a separate parameter for every internal algorithmic variable. Expose only meaningful user controls and carefully selected advanced settings.
Deliverable: A reliable parameter and state-management system.
Step 5. Establish the DSP testing infrastructure
Before implementing the mastering chain, create a test harness for audio processing.
Generate synthetic signals including:
- Silence.
- Impulses.
- Sine waves.
- Swept sine waves.
- Pink noise.
- Signals with deliberately excessive low frequencies.
- Transient-heavy signals.
- Wide stereo signals.
- Anti-phase stereo signals.
Add test utilities for measuring RMS, peak levels, spectral energy and stereo characteristics.
Test the algorithms independently of the plugin interface.
Include checks for NaNs, infinities, unexpected gain explosions and unstable filter states.
Deliverable: A reproducible way to measure and test DSP behaviour.
Step 6. Implement reference loading and analysis management
Create the reference-loading component.
It should support file selection and drag-and-drop, decode supported audio formats, and extract the information necessary for analysis.
Reference decoding and full-track analysis must run outside the real-time audio callback.
Implement a background analysis worker and a safe way to provide completed analysis profiles to the audio engine.
The reference manager should handle:
- Loading and unloading references.
- Invalid or unsupported files.
- Analysis progress.
- Cached analysis results.
- Reference changes.
- Saving and restoring reference metadata.
Do not perform disk access or lengthy analysis operations in the audio callback.
Deliverable: A reliable reference-loading subsystem that does not interfere with audio playback.
Phase 3 — The analysis engine
Step 7. Implement spectral analysis
Build the spectral analysis engine before the matching algorithms.
The engine should calculate:
- Energy and RMS per frequency band.
- Relative spectral balance.
- A finer-resolution spectral representation for EQ matching.
- Spectral changes over time.
- Narrow frequency concentrations that may indicate resonances.
Keep the octave-band representation as the main interface view.
Internally, use a finer spectral resolution when required for precise equalization.
Deliverable: A tested Source Analyzer capable of generating spectral profiles.
Step 8. Implement dynamics and stereo analysis
Add calculations for:
- Global integrated and short-term loudness.
- True Peak.
- Per-band RMS and peak levels.
- Crest factor.
- Envelope and transient characteristics.
- Mid/Side energy.
- Stereo correlation.
- Width by frequency band.
Use clearly defined measurement methods. In particular, do not present ordinary per-band RMS measurements as standardized LUFS measurements.
Compare the results against known test signals to validate the implementation.
Deliverable: A complete feature extractor for the measurements required by the first version.
Step 9. Implement section analysis
A single average over an entire track is insufficient for reliable reference matching.
Add temporal statistics and, where practical, automatic segmentation.
Allow the user to select representative passages manually.
For example, the plugin should be able to compare a drop in the Source with a representative drop in the Reference instead of comparing two completely different arrangements.
The system should also support whole-track comparison when the structures differ.
Deliverable: Global and section-based analysis profiles.
Step 10. Implement the Matching Engine without modifying audio
This is the most important algorithmic step.
Initially, the Matching Engine should only calculate and display proposed corrections.
It should determine:
- Which spectral differences warrant EQ.
- Which dynamic differences warrant compression.
- Whether the transient characteristics justify processing.
- Whether the stereo width should change.
- Whether a resonance reduction is justified.
- Whether the loudness target is achievable within the processing limits.
Each proposed correction should have a confidence estimate and bounded parameters.
The engine should also recognize when a difference is probably caused by the arrangement rather than a correctable mastering issue.
Do not apply any of the proposed corrections yet.
Test the decisions against your reference dataset and inspect whether the engine is recommending sensible interventions.
Deliverable: A functioning analysis-and-decision system, independent of the DSP chain.

Step 10b. Implement the Mix Advisor (source-level guidance)
Some differences cannot be safely fixed in a master (already-squashed transients, mono or phase problems, static low-mid buildup, missing arrangement content). The Mix Advisor reports such cases as small, actionable suggestions the artist can fix at source before re-sending the track, instead of over-processing the master.
It is a second consumer of the Step 10 difference/confidence engine: the Matching Engine asks "can I fix this within bounds?", the Mix Advisor asks "is this better fixed at source?". It never modifies audio and never blocks the automatic master.
Honesty constraints: a stereo master cannot identify instruments, so never assert an instrument name; emit a ranked list of plausible sources with a confidence and "verify by ear". Use frequency x temporal structure (percussive vs sustained) and persistence, not frequency alone, to rank sources. Keep magnitudes small and reversible ("+1-2 dB", "slow the attack"). Stay silent when evidence is weak, and level-normalise Source and Reference before comparing.
Initial rules: R1 low-end punch deficit, R2 low-end too wide/phase, R3 over-compressed at source, R4 low-mid buildup, R5 dull/missing air, R6 harsh presence, R7 narrow image, R8 dull high transients, R9 resonant bass/synth, R10 kick/bass conflict, R11 phase problem. Details in Docs/PRODUCT_SPEC.md section 5.5.
Deliverable: A deterministic, bounded, confidence-tagged advice module, separate from the render chain.
Phase 4 — The first functional mastering plugin
Step 11. Implement EQ matching
Implement the first processing module that modifies the audio.
Use the spectral profiles calculated in the previous steps to estimate the EQ corrections required to approach the reference.
The implementation must include:
- Spectral smoothing.
- Bounded boosts and cuts.
- A maximum number of correction filters.
- Parameter smoothing where necessary.
- Protection against excessive high-frequency or low-frequency correction.
Evaluate the resulting audio against the unprocessed Source.
At this stage, prioritize stable, useful tonal matching over reproducing every detail of the reference spectrum.
Deliverable: The first genuinely useful automatic mastering processor.
Step 12. Implement low-frequency management
Add the low-frequency M/S processing module.
It should estimate whether reducing the Side component below approximately 90–100 Hz is appropriate.
Implement configurable frequency transition, filtering slope and processing intensity.
Include a strict mono mode, but do not enable it indiscriminately.
Measure the effect on stereo width and the summed mono signal.
Deliverable: A configurable low-frequency management module.
Step 13. Implement the output stage
Add output gain control and a True Peak limiter.
The limiter should support a configurable peak ceiling, with −1.0 dBTP as the default target.
Implement the loudness-targeting logic separately from the A/B listening compensation.
The Matching Engine should determine whether the reference loudness can be approached without unacceptable loss of dynamics.
If it cannot, it should respect the configured processing limits and report the remaining difference.
Deliverable: A complete basic mastering chain.
Step 14. Implement A/B monitoring and the first dashboard
Now build the interface around the working analysis and processing engine.
Implement:
- Reference loading.
- Source versus Reference octave-band visualization.
- Before/after A/B switching.
- Level-matched listening.
- Delta monitoring.
- Match Intensity from 0% to 200%.
- Processing enable/disable controls.
- Analysis status and error reporting.
The Match Intensity control should scale each type of correction appropriately instead of simply multiplying every DSP parameter.
At 0%, the audio should pass through unchanged unless the user has explicitly enabled independent processing.
Deliverable: The first usable version of the plugin.
At this point, you should already be able to load a reference, analyze a Source, apply tonal matching and compare the result with the original.
This is the first version worth testing extensively on real electronic music.
Phase 5 — Advanced mastering algorithms
Step 15. Implement dynamic matching
Add the global glue compressor and multiband compressor.
Use the analysis results to determine whether compression is appropriate and which bands require it.
Each compressor must retain its own envelope detector.
Do not create a feedback loop in which the Matching Engine constantly adjusts the compression parameters in response to individual meter updates.
Instead, calculate sensible parameters from the analysis, configure the processors and evaluate the resulting audio.
Add explicit processing limits and monitor gain reduction.
Deliverable: Dynamic matching that complements the EQ matching without unnecessarily sacrificing transients.
Step 16. Implement transient shaping
Add the transient shaper using the previously calculated transient statistics.
The module should estimate the proéminence of transient content, not claim that it can reliably isolate individual drums from an arbitrary stereo master.
Test it on different electronic genres, including material with plucks, sharp synth attacks and heavily compressed drums.
Ensure that the system can decide not to intervene.
Deliverable: Controlled transient matching.
Step 17. Implement the Smart De-Resonator
Develop the resonance-detection algorithm separately from the EQ matcher.
Evaluate frequency concentrations according to their strength, bandwidth, persistence and behaviour over time.
Only apply reductions when the detection is sufficiently reliable.
Test false positives, especially on harmonic synths, resonant basses and deliberately bright electronic sounds.
Deliverable: A conservative dynamic resonance-reduction module.
Step 18. Implement adaptive saturation
Add global and multiband saturation as optional modules.
Compensate for the change in output level when comparing different saturation settings.
Measure the added distortion, changes in spectral balance and effect on the output peaks.
Do not infer that saturation is required simply because the Reference has a different harmonic spectrum.
Deliverable: Optional saturation with predictable, bounded behaviour.
Step 19. Implement advanced stereo matching
Extend the stereo processor to match width across the different frequency bands.
Use Mid/Side measurements, correlation and mono-compatibility checks to avoid excessive widening or unstable low frequencies.
Evaluate the effects of all stereo processing modes on the final output.
Deliverable: A stereo-matching module that complements the low-frequency processor.
Phase 6 — Integration and release
Step 20. Integrate the complete processing chain
Now integrate all modules under the control of the Matching Engine.
Validate the interaction between:
- Saturation and EQ.
- Transient shaping and compression.
- Multiband compression and limiting.
- EQ and resonance reduction.
- Stereo widening and mono compatibility.
- Loudness targeting and True Peak limiting.
The Matching Engine must evaluate the effect of the complete chain, rather than treating each processor as an independent correction system.
Add a penalty for unnecessary or excessive processing.
Deliverable: A coherent automatic mastering chain.
Step 21. Test the plugin systematically
Use automated tests and actual DAW sessions.
Test:
- Multiple sample rates and audio block sizes.
- Silence and extreme input signals.
- Different electronic genres.
- References with different loudness levels.
- Identical Source and Reference signals.
- References with very different arrangements.
- Missing reference files.
- Parameter automation.
- State save and restore.
- Bypass.
- Real-time playback and offline rendering.
Use pluginval to validate the resulting plugin. It supports automated plugin testing, including VST3, and can be incorporated into a CI workflow. (pluginval documentation)
Compare the processed result against the Source at matched listening levels.
A reduction in measured spectral error is not sufficient evidence of improved sound quality.
Deliverable: A documented set of tests and a stable plugin build.
Step 22. Package and distribute
Prepare the release build.
Complete:
- Installation and compatibility testing.
- GUI and parameter-state testing.
- CPU and memory profiling.
- DSP regression tests.
- Third-party licence review.
- Versioning and release notes.
- User documentation.
- A reproducible build procedure.
Add other plugin formats and platforms only after the first supported build is reliable.
Deliverable: A versioned, installable plugin suitable for distribution.
Recommended first milestone
Do not attempt to build the complete specification in one pass.
Your first objective should be:
1. A working VST3.
2. Reference loading.
3. Full-track Source and Reference analysis.
4. Octave-band visualization.
5. An independent Matching Engine.
6. EQ matching.
7. Low-frequency M/S control.
8. True Peak limiting.
9. A/B monitoring with level compensation.
Once this version produces consistently useful results, add the more difficult algorithms.
The project's value will depend primarily on the quality of its matching decisions, not on the number of processors in the chain.
