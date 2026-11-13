# openClapper v3 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use $subagent-driven-development (recommended) or $executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rebuild openClapper as a trance-focused procedural clap instrument whose tuned body follows MIDI pitch directly, with host-synced slapback and a fully redesigned factory bank.

**Architecture:** `ClapSynth` generates one dry, MIDI-tuned clap voice. `ClapEngine` allocates eight voices, sums their output, and routes it through one host-synced slapback tap, the existing reverb and output effects. APVTS parameters and the existing panels expose the new controls; all 44 presets define new trance timbres without a root note.

**Tech Stack:** C++20, JUCE 8.0.9, CMake 3.22+, existing GitHub Actions workflow.

**Spec:** `docs/superpowers/specs/2026-11-13-openclapper-v3-design.md`

## Global Constraints

- Map MIDI note numbers 0–127 directly to `440 * 2^((note - 69) / 12)`.
- MIDI note 69 maps to A4 at 440 Hz; note 57 maps to A3 at 220 Hz and note 81 maps to A5 at 880 Hz.
- Keep overlapping hits alive with a bounded eight-voice pool; when all voices are busy, reuse the oldest voice and let already-emitted delay and reverb tails continue.
- Keep all 44 factory slots and replace every preset with a trance-focused design.
- Slapback divisions are Off, 1/16, dotted 1/16, 1/8, dotted 1/8, and 1/4; use one repeat without feedback.
- If the host supplies no valid tempo, use the engine's existing 138 BPM fallback.
- Keep JUCE 8.0.9 and C++20; add no sample library or external audio dependency.
- Retain the current interface style and panel layout; remove pitch-selection controls and add slapback controls in the effects area.
- Build the Release plugin locally; do not add new test cases or run tests locally. The existing GitHub workflow runs its current tests on release pushes.
- Set the project and UI version to 3.0.0; push `main` and tag `v3.0.0` using the personal noreply git identity.

## Review Focus

- MIDI note 69 and adjacent notes must keep the tonal fundamental at 440 Hz and the expected equal-temperament intervals. Check the frequency helper and note passed from `ClapEngine` to each `ClapSynth` voice.
- MIDI notes near 0 and 127 must remain finite and avoid aliased partials. Check frequency clamping and partial omission in the synth implementation.
- A note arriving while all eight voices are active must steal the oldest voice without clearing the global delay/reverb state. Check the allocator and the core build.
- Each slapback division must compute its duration from BPM, including the 138 BPM fallback. Check the division-to-beat mapping and the existing CI build/test workflow.
- Multiple MIDI events in one audio block must trigger at their sample offsets and preserve their note numbers and velocities. Check `PluginProcessor::processBlock` and `ClapEngine::process`.

---

## File Map

- `Source/Core/ClapSynth.h` and `Source/Core/ClapSynth.cpp`: replace root-note tuning with direct MIDI-note voice input; produce dry transient/noise and tonal layers.
- `Source/Core/ClapEngine.h` and `Source/Core/ClapEngine.cpp`: own eight voices, sample-offset MIDI dispatch, shared reverb, voice summing, and final effect order; remove fixed-root and WAV lock playback paths.
- `Source/Core/Effects.h` and `Source/Core/Effects.cpp`: add the one-tap stereo slapback division enum and delay processor; own the shared gated reverb, distortion, compressor, and limiter algorithms.
- `Source/Plugin/Parameters.h` and `Source/Plugin/Parameters.cpp`: remove pitch/root/lock parameters and add slapback division and wet level; map APVTS state to the new core settings.
- `Source/Plugin/Panels.h` and `Source/Plugin/Panels.cpp`: remove the note/octave/fine/lock and MIDI-mode controls; expose slapback division and mix in the effects panel.
- `Source/Plugin/PluginProcessor.cpp`: pass MIDI note-on data directly, remove lock-WAV state serialization and preserve-pitch preset logic, and keep audition at A4.
- `Source/Plugin/Presets.cpp` and `Source/Plugin/Presets.h`: replace all 44 names and sound values; remove pitch-preservation behavior and assign host-sync divisions to slapback presets.
- `Tests/ClapSynthTests.cpp`, `Tests/ClapEngineTests.cpp`, and `Tests/PluginTests.cpp`: adapt existing cases to removed APIs and retired lock features only; add no test cases.
- `README.md`, `CMakeLists.txt`, and `Source/Plugin/PluginEditor.cpp`: document the v3 MIDI and sync behavior and set version strings to 3.0.0.

## Tasks

### Task 1: Replace the synth voice with a MIDI-tuned procedural clap

**Files:**
- Modify: `Source/Core/ClapSynth.h`
- Modify: `Source/Core/ClapSynth.cpp`
- Modify: `Source/Core/ClapEngine.cpp` only to adapt its existing synth trigger call
- Modify: `Tests/ClapSynthTests.cpp` only where existing calls use removed pitch fields or signatures

**Interfaces:**
- Produces: `float midiNoteToHz(int midiNote) noexcept`, returning `440.0f * pow(2.0f, (midiNote - 69) / 12.0f)`.
- Produces: `ClapSynth::trigger(const ClapSynthSettings& settings, int midiNote, float velocity) noexcept`.
- Produces: dry stereo frames from `ClapSynth::processSample`; no per-voice reverb.

- [x] Replace note/octave/fine-cent synthesis fields with a MIDI note number captured by `trigger`.
- [x] Build the voice from a stable fundamental, tuned partials below Nyquist, separately shaped noise bursts, and an independent transient layer. Keep the fundamental fixed to the captured MIDI note for the tonal decay.
- [x] Adapt the current engine call from relative semitones to MIDI note `69 + semitoneOffset`; Task 3 replaces that temporary conversion with the event's raw MIDI note.
- [x] Update the existing pitch-oriented core test calls to the MIDI-note API without adding test cases.
- [x] Build the core target: `cmake --build build --target openclapper_core --config Release --parallel`.
- [x] Commit the completed core voice with the personal git identity (`50dcf65`).

### Task 2: Add shared host-synced slapback processing

**Files:**
- Modify: `Source/Core/Effects.h`
- Modify: `Source/Core/Effects.cpp`
- Modify: `Source/Core/ClapEngine.h`
- Modify: `Source/Core/ClapEngine.cpp`
- Modify: `Source/Plugin/Parameters.cpp`
- Modify: `Source/Plugin/Parameters.h`
- Modify: `Source/Core/ClapSynth.h` and `Source/Core/ClapSynth.cpp` to make the shared reverb gate triggerable without clearing its tail state

**Interfaces:**
- Produces: `enum class SlapbackDivision { Off, Sixteenth, DottedSixteenth, Eighth, DottedEighth, Quarter }`.
- Produces: `SlapbackDelay::prepare(double sampleRate)`, `reset()`, and `process(float& left, float& right, SlapbackDivision division, float mix, double bpm) noexcept`.
- Consumes: dry stereo voice sum before shared reverb and output effects.

- [x] Map the divisions to 0, 0.25, 0.375, 0.5, 0.75, and 1.0 beats; compute delay samples from `beats * 60 / bpm * sampleRate`, with invalid BPM replaced by 138.
- [x] Implement one stereo tap with dry/wet mixing and no feedback; preserve delay tails across new note triggers.
- [x] Move the existing reverb settings into `ClapEffectSettings` and keep their APVTS parameter IDs; route voice output through slapback, then one shared `GatedStereoReverb`, then the existing distortion/dynamics/master path.
- [x] Build the core target: `cmake --build build --target openclapper_core --config Release --parallel`.
- [x] Commit the effect and engine routing with the personal git identity (`5db0ca7`).

### Task 3: Route MIDI note-ons through an eight-voice engine

**Files:**
- Modify: `Source/Core/ClapEngine.h`
- Modify: `Source/Core/ClapEngine.cpp`
- Modify: `Source/Core/ClapSynth.h` and `Source/Core/ClapSynth.cpp` to render reverse swells at the raw MIDI note
- Modify: `Source/Plugin/PluginProcessor.cpp`
- Modify: `Source/Plugin/Parameters.cpp` to remove retired engine settings
- Modify: `Tests/ClapEngineTests.cpp` and `Tests/ClapSynthTests.cpp` only to retire removed lock and offset APIs

**Interfaces:**
- Consumes: `ClapSynth::trigger(settings, midiNote, velocity)` and `SlapbackDelay` from Tasks 1 and 2.
- Produces: sample-offset note-on dispatch to one of eight `ClapSynth` voices, with oldest-voice stealing when all are active.

- [x] Allocate and prepare eight voices; record trigger order so the oldest active voice can be selected when all are busy.
- [x] Dispatch every `MidiTrigger` at its event sample and pass its note number and velocity unchanged; leave audition on MIDI note 69.
- [x] Remove Fixed/Note Tracking and WAV Lock In from engine settings and saved plugin state, including the WAV playback/cache path and obsolete lock test.
- [x] Build all configured targets: `cmake --build build --config Release --parallel`.
- [x] Commit the MIDI and voice allocation changes with the personal git identity (`2803242`).

### Task 4: Replace pitch controls with MIDI-only behavior and slapback controls

**Files:**
- Modify: `Source/Plugin/Parameters.h`
- Modify: `Source/Plugin/Parameters.cpp`
- Modify: `Source/Plugin/Panels.h`
- Modify: `Source/Plugin/Panels.cpp`
- Modify: `Source/Plugin/PluginProcessor.cpp`
- Modify: `Source/Plugin/PluginProcessor.h`, `Source/Plugin/Presets.h`, and `Source/Plugin/Presets.cpp` to remove lock telemetry and pitch preservation
- Modify: `Tests/PluginTests.cpp` only to retire cases for removed pitch-lock and octave controls

**Interfaces:**
- Produces: APVTS `slapback_division` choice with six entries and `slapback_mix` float in `[0, 1]`.
- Consumes: engine slapback settings from Task 2.

- [x] Remove `tone_note`, `tone_octave`, `tone_fine`, `tone_freq`, `midi_mode`, `lock_pitch`, and `lock_in` parameters and their attachments. `tone_freq` is currently unused by the audio path.
- [x] Add division and mix parameters and map them to `EngineSettings`.
- [x] Place the division selector and mix control in the existing effects panel without changing the overall window size or visual style.
- [x] Remove preset pitch preservation from `OpenClapperProcessor::setCurrentProgram` and update existing plugin tests only for the retired controls.
- [x] Build all configured targets: `cmake --build build --config Release --parallel`.
- [x] Commit the parameter and panel changes with the personal git identity (`1cc5b10`).

### Task 5: Rebuild all presets and update v3 documentation/versioning

**Files:**
- Modify: `Source/Plugin/Presets.cpp`
- Modify: `Source/Plugin/Presets.h` if the `apply` signature changes
- Modify: `Source/Plugin/Parameters.cpp` to set the preset parameter range to 44 programs
- Modify: `README.md`
- Modify: `CMakeLists.txt`
- Modify: `Source/Plugin/PluginEditor.cpp`

- [x] Replace all 44 preset names and values with distinct trance-focused designs; no preset sets a MIDI root or fixed note.
- [x] Set slapback mix and division in presets that use the effect; default other presets to Off.
- [x] Rewrite README MIDI behavior, synth description, slapback parameters, preset list, and build/release version references.
- [x] Set CMake and UI display versions to 3.0.0.
- [x] Build all configured targets: `cmake --build build --config Release --parallel`.
- [x] Commit presets, documentation, and version changes with the personal git identity (`c9876d5`).

### Task 6: Build, push, release, and track completion

**Files:**
- Modify: `Source/Core/ClapSynth.h` and `Source/Core/ClapSynth.cpp` to place pre-bursts before the main hit and report main-hit onset.
- Modify: `Source/Core/Effects.h` and `Source/Core/Effects.cpp` to carry delayed trigger markers to the shared reverb.
- Modify: `Source/Core/ClapEngine.cpp` to reset shared effect tails for CC120/123 and retrigger reverb on main hits and delayed repeats.
- Modify: `Source/Plugin/PluginProcessor.h` and `Source/Plugin/PluginProcessor.cpp` to report the active patch's tail duration.
- Modify: `README.md` to remove the stale v1 screenshot and describe the updated effect behavior.

- [ ] Review the full diff against `origin/main`; run `git diff --check` and confirm only requested files changed.
- [x] Configure a Release build if needed: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DOC_BUILD_PLUGIN=ON -DOC_BUILD_TESTS=ON` (the existing build was auto-reconfigured after the version update).
- [x] Build the plugin and test executables without running tests: `cmake --build build --config Release --parallel`.
- [x] Confirm the build exits successfully and inspect its complete output (exit code 0; tests not run).
- [ ] Before pushing, run `git log --format='%h %ae %ce' origin/main..HEAD`; every author and committer email must be `221119764+alanmarcero@users.noreply.github.com`.
- [ ] Push `main` and tag `v3.0.0` to `origin`.
- [ ] Monitor the GitHub Actions run and release at [issue #1](https://github.com/alanmarcero/openClapper/issues/1); close the issue only after the release workflow succeeds.
