# Testing on Windows (no agent needed)

This covers what exists today: the **core library**, the **unit + integration tests**, and the
**offline CLI**. There is **no VST3 plugin yet** — that is the next phase. Nothing here needs
OpenCode; you only need the tools below.

## 1. Prerequisites (install once)

- **Git for Windows** — https://git-scm.com/download/win
- **CMake 3.24 or newer** — https://cmake.org/download/ (tick "Add CMake to PATH")
- **Visual Studio 2022** with the **"Desktop development with C++"** workload
  (this provides the MSVC compiler and the `cmake --preset windows-msvc` generator).
- Optional: **Python 3** — only needed for the `style_rules` test. Without it, that one test
  is simply not registered and `ctest` still runs the unit tests.

Verify in a fresh **Developer PowerShell for VS 2022**:

```powershell
git --version
cmake --version
```

## 2. Get the code

```powershell
git clone https://github.com/Thibaut-Fontaine/masterizer.git
cd masterizer
```

(If the repo is private, use your GitHub credentials, or `git@github.com:...` with your SSH key.)

## 3. Build

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-msvc --config Release
```

Or just run the helper: `./scripts/build.ps1`

## 4. Run the tests

```powershell
ctest --preset windows-msvc
```

**Expected result:** `100% tests passed`, with one test named `unit` (about 211 cases) and,
if Python 3 is installed, a second named `style_rules`. Individual test binary:

```powershell
./build/windows-msvc/tests/Release/masterizer_tests.exe
```

## 5. Run the offline mastering tool

It takes a **reference** file and a **source** file and writes a mastered **output** WAV.
Inputs may be **WAV** (16-bit PCM / 32-bit float), **FLAC** or **MP3** (any mix, e.g. MP3
reference + FLAC source).

```powershell
./build/windows-msvc/app/Release/masterizer_cli.exe reference.wav source.wav output.wav
```

Options:

```text
--intensity N     Match Intensity 0..2 (default 1)
--ceiling dB      True Peak ceiling (default -1)
--gain dB         output gain (default 0)
--ab              also write a level-matched <output>.ab.wav  (fair A/B comparison)
--delta           also write <output>.delta.wav  (processed minus source)
--<module> / --no-<module>
    modules: eq mono limiter saturation tape resonance stereo transient glue multiband dynamic-eq
```

Example output line:

```
wrote output.wav | proposals=3 | LUFS=-12.82 target=-11.60 shortfall=1.22 LU | truePeak=-6.38 dBTP
```

### How to judge (important)

- Compare `output.wav` against the level-matched `output.ab.wav` — **not** against the raw
  source (louder always "sounds better").
- Listen to `output.delta.wav` to hear *only* what changed.
- Isolate modules one at a time: `--no-eq`, `--no-monobass`, `--no-limiter`, `--resonance`, etc.
- Try `--intensity 0.5`, `1`, `1.5` to hear how strongly the match is applied.
- A shortfall > 0 means the reference's loudness was **not** reached (by design, rather than
  over-limiting); note whether that seems right for the track.

What the line means:
- `proposals` — how many corrections the matching engine proposed.
- `LUFS` vs `target` — achieved loudness vs the reference's loudness; `shortfall` is how far
  short it stayed (0.00 means the target was reached).
- `truePeak` — the render's True Peak (should be at or below the ceiling, default -1.0 dBTP).

## 6. What to report back

Please send:
1. The exact build output if it **fails** (paste the error text).
2. The `ctest` summary line(s).
3. The CLI output line, for a couple of real tracks.
4. Whether the output sounds like a **reasonable** mastering pass — better, worse, or broken.
   Synthetic tests prove the code runs; only your ears can say if it is any good.

## 7. Known limitations (expected, not bugs)

- No plugin/UI yet — this is the offline engine only.
- **Keep paths ASCII** (no accented/emoji characters): the MP3/FLAC decoders use narrow
  `fopen`, so non-ASCII Windows paths may not open. ASCII paths work on both platforms.
- Loudness targets may be missed (a shortfall is reported rather than over-limiting).
- Dynamics modules are per-channel (no stereo link yet); the limiter controls sample peak and
  a separate True Peak guard catches inter-sample peaks.
- Every threshold is a documented initial guess, not calibrated on real music.
