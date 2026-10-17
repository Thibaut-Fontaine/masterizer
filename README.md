# Masterizer

Reference-driven automatic mastering VST3 for electronic music. The user loads a
finished reference track; the plugin analyses the Reference and the DAW Source and
proposes/applies a coordinated mastering chain. The Reference is a **target profile,
not a command** — the plugin must be willing to do nothing, correct less, or report
that a target cannot be reached safely.

See `AGENTS.md` for working rules and `Docs/PRODUCT_SPEC.md` / `Docs/ROADMAP.md`
for the binding requirements and implementation order.

## Architecture (layered MVC + Ports & Adapters)

- **Model** — `include/masterizer/domain/**`, `src/domain/**`: pure C++20, no JUCE,
  100% unit-tested (DSP, analysis, matching, state).
- **Controller** — `include/masterizer/application/**`, `src/application/**`:
  use-case orchestration (Analyze & Apply, Live Match, A/B + Delta, output validation).
- **View / Adapters** — `src/plugin/**`: thin JUCE layer (`PluginProcessor`,
  `PluginEditor`, `ParameterLayout`). No DSP logic lives here.
- **Tests** — `tests/unit/**` (mirrors the domain/application tree), `tests/support/**`
  (tiny in-repo harness), `tests/integration/**`.

Headers live in `include/masterizer/<layer>/`, implementations mirror them under
`src/<layer>/`. CMake is the build system: it generates Makefiles on Linux/macOS and a
Visual Studio solution on Windows from the same source.

## Toolchain

- C++20; JUCE 8.0.15 (pinned); VST3; CMake >= 3.24.
- Primary target: Windows 10 1607+ (x64), Visual Studio 2022. The core also builds
  and tests on Linux (GCC/Clang) and macOS.

## Build and test

Linux / macOS:

```sh
cmake --preset linux-debug
cmake --build --preset linux-debug
ctest --preset linux-debug
```

Windows (MSVC):

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-msvc
ctest --preset windows-msvc
```

Run a single test by name substring (harness):

```sh
./build/linux-debug/tests/masterizer_tests rms
```

## Offline render tool

```sh
./build/linux-debug/app/masterizer_cli <reference> <source> <output.wav> [options]

# both delivery profiles in one run:
./build/linux-debug/app/masterizer_cli ref.mp3 src.flac out.wav --profile both
# -> out.soft.wav (-1.0 dBTP, safe) and out.loud.wav (-0.1 dBTP, hot)
```

Reads a reference and a source file (WAV, FLAC or MP3), analyses both, proposes and applies
corrections, **raises loudness toward the reference** (up to +12 dB), limits to the True Peak
ceiling, and writes the result (WAV). Prints proposals, applied gain, LUFS vs target, shortfall
and true peak. (The VST3 plugin is still to come.)

## Non-negotiable size rules

Every C++ source file must satisfy, and `scripts/check_rules.py` enforces (wired as the
`style_rules` CTest test):

1. at most **5 functions** per file,
2. at most **60 lines** per function body,
3. at most **110 characters** per line.

## Development method

Strict TDD: write the failing unit test first, run it red, implement the minimum, run it
green, then move on. Keep each change a single buildable step from `Docs/ROADMAP.md`.
