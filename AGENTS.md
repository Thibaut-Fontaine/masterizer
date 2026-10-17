# AGENTS.md

Agent-facing guidance for this repo. Read this first, then the linked docs.

## What this repo is

A reference-driven automatic mastering VST for electronic music: the user loads a
finished reference track, the plugin analyses the Reference and the DAW Source, and
proposes/applies a coordinated mastering chain. The Reference is a **target profile,
not a command** — the product must be willing to leave things alone or report that a
target cannot be reached safely.

**Current state: core + UI-logic (Phase 1) complete.** The library (measurement, analysis,
matching, Mix Advisor, full bypassable processing chain, coordination, output validation), the
offline `masterizer_cli`, unit + integration tests, docs, and the JUCE-free **UI logic**
(`DashboardModel` presenter + `Parameters` model) exist. Remaining: the **JUCE VST3 shell**
(Phase 2 — pass-through plugin, parameters/state, editor bound to `DashboardModel`), the
decode adapter and the dashboard/"Source notes" UI, plus real-world validation.
See `Docs/WINDOWS_TESTING.md` to build/test on Windows without an agent.

## Source-of-truth docs

- `Docs/PRODUCT_SPEC.md` — binding requirements: octave-first analysis, matching-engine
  responsibilities, per-module DSP specs, architecture boundaries, validation matrix.
  Do not dilute these requirements. If something is deferred, keep it in the roadmap
  with a concrete implementation/test plan rather than dropping it.
- `Docs/ROADMAP.md` — the required implementation order (22 steps / stages 0–10).
  Build **one buildable step at a time**; each step must build and be verifiable
  before the next. Do not implement the full chain in one pass.

## Locked technical decisions

- C++20, **JUCE 8.0.15** (pinned; do not auto-upgrade), CMake ≥ 3.24, **VST3**.
- Primary target: **Windows 10 1607+ x64** (Visual Studio 2022). The core also builds on
  Linux/macOS. VST2 is intentionally dropped (Steinberg removed the SDK); do not add it.
- Architecture: layered MVC + Ports & Adapters. Model = `include|src/masterizer/domain/**`
  (pure C++20, no JUCE, fully unit-tested); Controller = `application/**`; View/adapters =
  `src/plugin/**`. Headers live in `include/masterizer/<layer>/` and mirror `src/<layer>/`.
  Domain must never include JUCE; UI must never touch DSP internals directly.
- The test harness is **in-repo and dependency-free** (`tests/support/**`). Do not vendor a
  third-party test framework (doctest/Catch2/etc.): the ≤5-functions rule must apply to
  every file with no exceptions.
- macOS support is a bonus, not a requirement; Windows is the primary target.

## Build and test (verified)

```sh
cmake --preset linux-debug
cmake --build --preset linux-debug
ctest --preset linux-debug
```

Windows: use `--preset windows-msvc`. Run one test by name substring:
`./build/linux-debug/tests/masterizer_tests <substring>`.

## Non-negotiable rules (enforced)

- **TDD**: write the failing test first, run it red, implement the minimum, run it green.
  Never advance to the next roadmap step with a red test.
- Every C++ file: **≤ 5 functions**, **≤ 60 lines per function body**, **≤ 110 columns
  per line**. `scripts/check_rules.py` enforces this as the `style_rules` CTest test; a
  red style check blocks progress like a failing unit test. Function length assumes the
  opening brace sits on the signature line.
- **Commit on green**: once a step's unit tests *and* `style_rules` pass, commit with a
  clear message and `git push origin main`. Never commit a red build.

## Git

- Remote `origin` is `git@github.com:Thibaut-Fontaine/masterizer.git` (**SSH**; HTTPS has
  no credentials in this environment). Branch: `main`.

## Hard rules (easy to violate)

- Audio callback: no disk I/O, decoding, locks, allocation, or logging. Prepare off-thread
  and hand results over via immutable analysis profiles.
- Never claim a build, test, DAW load, or listening test passed unless you actually ran it
  and inspected the result. Never claim a later stage is implemented when only a stub exists.
- Octave bands are the **UI/summary** model only; use finer log bins / STFT internally for EQ
  and resonance work.
- Never label per-band RMS as standardized LUFS. Full-signal LUFS and True Peak need
  documented methods.
- Match Intensity: `0%` = neutral passthrough; `100%` = proposed corrections; `>100%`
  strengthens via module-specific curves — never multiply all DSP parameters blindly.
- Meter readings must not drive processor parameters frame-by-frame; the Matching Engine
  decides from profiles, then processors run with their own detectors.
- Do not retain full decoded reference audio in memory. Cache versioned analysis profiles
  and invalidate the cache when analysis behaviour changes.

## Mix Advisor (Stage 5b)

Source-level guidance: reuse the Stage 5 difference/confidence engine to emit small,
actionable suggestions the artist can fix at source. Full spec in `Docs/PRODUCT_SPEC.md`
section 5.5; roadmap Step 10b.

- Advice only, off-thread, **never in the render chain**; never gates or delays the auto master.
- **Never name an instrument as fact.** Emit a ranked plausible-source list + confidence +
  "verify by ear". Frequency alone is not enough: use frequency **x** temporal structure
  (percussive vs sustained) and persistence.
- Keep magnitudes small and reversible; stay silent when evidence is weak; level-normalise
  Source and Reference before comparing.
- Output is versioned by `AdvisorRuleset`; bump that version whenever rules or thresholds change.
- Rules R1-R11 (low-end punch, low-end width/phase, source over-compression, low-mid buildup,
  missing air, harsh presence, narrow image, dull high transients, resonant bass, kick/bass
  conflict, phase) are a data table, not hard-coded branches.

## Ordering gates (do not skip)

Work only the earliest unmet gate; never build a later gate while an earlier one is unmet.

- **Gate 0 — engine built + unit/integration tested.** Done.
- **Gate 1 — the engine makes sensible sound on real music** (offline CLI + ears). **Unmet; current focus.**
- **Gate 2 — plugin/host integration** (JUCE VST3, decode adapter, dashboard). Only after Gate 1.
- **Gate 3 — release quality** (validation matrix, packaging, licences).

Do NOT start JUCE before Gate 1 passes: the CLI already makes the engine audible, so a plugin
adds nothing to validation and is the one part that cannot be compiled in the agent sandbox.

## Current backlog

`Docs/BACKLOG.md` is the live todo list. **First: step 0 — characterise the residual difference**
(section-wise bands, inter-harmonic/THD analysis on sustained lead notes, attack-envelope and
level-matched delta) before changing any DSP. Then the immediate items: **1)** dynamics guard in
loudness targeting, **2)** edge-band shelves (VHigh air, Sub), **3)** reduce the tape's HF
roll-off. Then EQ resolution, the real per-band stereo metric, a near-mono guard, a look-ahead
limiter, and performance. The VST shell comes last.

The speed plan is **S0–S7**, and every step has a mandatory checkpoint: green tests, a 3-run
median benchmark, one commit, and a short reflection in `Docs/REFLECTIONS.md`. No mixed
optimizations in one commit; the regression contract in the backlog must hold.

Metric roles are fixed there: **True Peak = hard constraint, LUFS = soft target, crest/LRA =
guards, width = optional/diagnostic**. Do not drift back to "match every metric".

## Working conventions
- **Show the proposed file/folder structure and plan before starting a new area, and wait
  for approval** — do not begin a restructure unprompted.
- **Install whatever you need yourself** (toolchain, CMake, packages) and document what was
  installed, rather than asking the user to install it.
- Before a substantial change, state the intended behaviour. After it, actually build and
  run the relevant tests, then report what passed, failed, or remains untested.
- Keep `AGENTS.md`, docs, build scripts and code consistent — update this file when commands,
  versions or layout change.
- Prefer simple, explicit DSP and data flow over speculative abstractions or new dependencies.
  Review third-party licences (JUCE licensing in particular) before distribution.
