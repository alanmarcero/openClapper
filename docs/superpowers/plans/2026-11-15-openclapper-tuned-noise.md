# openClapper Tuned Noise Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use $executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore MIDI-tuned noise excitation on openClapper Bandpass patches.

**Architecture:** Reuse the current MIDI fundamental and noise generator to feed a cached resonant bandpass in Bandpass mode. Leave the v3 partial oscillator in the other tone modes.

**Tech Stack:** C++20, JUCE 8, CMake, Catch2.

**Spec:** `docs/superpowers/specs/2026-11-15-openclapper-tuned-noise-design.md`

## Global Constraints

- Preserve the incoming MIDI note as the resonator frequency.
- Preserve non-Bandpass synthesis modes and all 44 factory preset definitions.
- Update the project to 3.1.0 and track the tagged CI release.

## Review Focus

- Bandpass output must depend on the selected noise type even when the broadband layer is muted.
- A change in MIDI note must retune the Bandpass resonator.
- `toneQ` changes must refresh cached coefficients without recalculating them every sample.
- All four tone modes must continue to render finite, audible output.
- All 44 factory programs must load, and release packaging must keep existing formats and identity.

---

### Task 1: Restore tuned noise excitation

**Files:** `Tests/ClapSynthTests.cpp`, `Tests/PluginTests.cpp`, `Source/Core/ClapSynth.h`, `Source/Core/ClapSynth.cpp`, `CMakeLists.txt`, `README.md`

**Interfaces:** Use the existing `ClapSynth::renderBlock()`, `ClapSynthSettings`, and MIDI note argument.

- [ ] Add a test requiring Bandpass output to differ by noise type when `noiseLevel` and pre-clap count are zero.
- [ ] Run the focused test and confirm it fails on the current v3 oscillator-only Bandpass path.
- [ ] Restore the Bandpass impulse/noise exciter at the current MIDI fundamental; cache coefficients by `toneQ` and preserve the other modes.
- [ ] Audit all 44 factory patches; add a render check for finite, non-silent tonal output across the bank.
- [ ] Run the full CTest suite and build VST3, AU, LV2, and Standalone.
- [ ] Review the diff, commit with the personal Git identity, push `main` and `v3.1.0`, then track the workflow to a successful release.
