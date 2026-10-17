# Testing

Strict TDD: write the failing test first, run it red, implement the minimum, run it green,
commit and push. Never advance with a red test.

## Commands

```sh
cmake --preset linux-debug        # or windows-msvc
cmake --build --preset linux-debug
ctest --preset linux-debug        # unit + style_rules

./build/linux-debug/tests/masterizer_tests <substring>   # one test by name
python3 scripts/check_rules.py .                          # size rules directly
```

## Harness

In-repo and dependency-free (`tests/support`): self-registering `TEST_CASE`, `CHECK` /
`REQUIRE` / `CHECK_NEAR`, run-with-filter, exit code. No third-party test framework, so the
size rules apply to every file.

## Size rules (enforced by `style_rules`)

1. at most 5 functions per file,
2. at most 60 lines per function body,
3. at most 110 columns per line.

A function definition is detected at column 0 with its opening brace on the signature line.

`includes` (also a CTest test) verifies every C++ file directly includes the standard headers
it uses. MSVC does not pull in `<vector>`/`<string>`/etc. transitively the way GCC/Clang often
do, so this catches Windows-only build failures before they happen.

`portability` (also a CTest test) rejects platform-specific constructs (`<unistd.h>`, `<sys/...>`,
`<windows.h>`, `__declspec`, `__attribute__`, `#pragma comment`, `M_PI`, `system()`).

CI (`.github/workflows/ci.yml`) builds and tests on **ubuntu-latest and windows-latest** on every
push/PR, so a change that only works on one platform fails visibly.

## Fixtures

`tests/support`: silence, impulse, sine, sweep, log pink noise, transient-rich, bass-heavy,
white noise, wide-stereo, anti-phase; plus `profileWithBands` and a fake `IAudioDecoder`.

## What is covered

- Every DSP/analysis/matching unit has focused unit tests.
- Integration tests (`tests/integration`): identical Source/Reference stays close, an
  unreachable target is reported, the full chain stays finite and under the ceiling, silence
  is safe.
- WAV round-trip and the offline CLI (smoke-tested).

## Not covered (yet)

Real music, listening tests, plugin validation (`pluginval`), a real DAW, host sample rates
and block sizes, automation, state restore, and CPU/real-time safety (no audio callback yet).
