# openClapper v3 MIDI-Pitched Trance Clap Design

**Status:** Approved for implementation
**Tracker:** [GitHub issue #1](https://github.com/alanmarcero/openClapper/issues/1)

## Intent

Rebuild openClapper as a trance-focused clap instrument whose tuned body follows incoming MIDI notes directly. A4 (MIDI note 69) must produce a 440 Hz tonal core, and other notes must follow standard equal temperament across the MIDI range. Factory presets should vary the clap's texture and space without choosing or locking its root pitch.

## Goals

- Map MIDI note numbers 0–127 directly to `440 * 2^((note - 69) / 12)`.
- Remove the separate root note, octave, fine tuning, MIDI mode, and lock-pitch workflow.
- Replace the current clap synthesis path with a new procedural engine combining a stable tuned body, layered transient/noise bursts, and stereo articulation.
- Keep overlapping MIDI-triggered hits alive so fast trance patterns do not cut off earlier clap tails. Use a bounded eight-voice pool.
- Redesign all 44 factory presets around trance styles and variations.
- Add a slapback tap with host-tempo note divisions and per-preset mix/division values.
- Release as v3.0.0 through the existing GitHub Actions workflow.

## Non-goals

- Add a sample library or external audio dependency.
- Redesign the plugin's visual identity. Retain the current chassis and panel style, removing pitch-selection controls and adding the slapback controls within the existing effects area.
- Add sustained MIDI note behavior; note-off remains unused because claps are one-shot sounds.

## Approaches considered

1. **Procedural synthesis (selected):** generate a stable MIDI-tuned tonal core and layer it with shaped noise, short slap clusters, and transient detail. This gives exact pitch mapping, keeps the instrument editable, and directly fulfills the request to rebuild the synthesis engine.
2. **Hybrid synthesis and samples:** use samples for the initial clap texture and synthesis for the pitched body. This can add realism, but requires authored sample assets and can make the attack change across pitch-shifted notes.
3. **Pitched sample bank:** trigger samples selected by MIDI note and pitch-shift between zones. This can sound realistic at a reference pitch, but changes transient timing/timbre across octaves and does not rebuild the synthesizer requested.

The procedural approach is selected because the requested instrument is a rebuilt synth and its pitch response must remain consistent across octaves.

## Design

### MIDI pitch

- Every MIDI note-on triggers at its received note number; there is no fixed-root mode or relative transposition from a UI-selected note.
- MIDI note 69 maps to A4 at 440 Hz; note 57 maps to A3 at 220 Hz and note 81 maps to A5 at 880 Hz.
- The tuned body keeps its fundamental at the MIDI frequency for its decay. Presets change its harmonic balance, envelope, and level without detuning that fundamental.
- MIDI velocity continues to scale the hit level.
- Remove the root note, octave, fine-cent, tone-frequency, MIDI-mode, and lock-pitch parameters and their controls. The current tone-frequency parameter is not connected to synthesis. Remove WAV Lock In as well, because frozen single-pitch playback would bypass live MIDI pitch tracking.
- Use eight overlapping clap voices. When all are busy, reuse the oldest voice; already-emitted delay and reverb tails continue independently.

### Synthesis engine

- Replace the current single-voice noise/tone implementation with a procedural layered clap voice.
- Build the tonal body from the exact MIDI fundamental and a small set of tuned modes. Clamp or omit modes above Nyquist so high MIDI notes do not alias.
- Build the slap from separately shaped noise bursts with per-burst amplitude, spacing, and stereo position. Keep the noise and click layers independent of the MIDI fundamental so they add impact without obscuring the requested pitch.
- Keep the current compressor, limiter, distortion, and gated reverb processors unless integration requires moving their call sites. Redesign their values across all presets.

### Host-synced slapback

- Add a single stereo slapback tap after the voice sum and before the existing reverb/dynamics path.
- Expose `Off`, `1/16`, dotted `1/16`, `1/8`, dotted `1/8`, and `1/4` divisions, plus a wet-level control.
- Compute delay samples from the host BPM and selected beat division. If the host supplies no valid tempo, use the engine's existing 138 BPM fallback.
- Use one repeat without feedback; this keeps the effect a slapback and avoids an uncontrolled tail. Presets that use it set both the division and mix.

### Presets and interface

- Keep 44 factory slots and replace every preset name and value with a trance-focused design spanning uplifting, progressive, psy, tech, classic gated, and minimal/club utility sounds.
- Presets may vary transient shape, tonal mode balance, noise spectrum, stereo spread, reverb, distortion, dynamics, and slapback division/mix. Presets cannot set a root note because pitch comes from MIDI.
- Retain the current interface style and overall layout. Remove note/octave/fine/lock controls, and add the slapback division and mix controls to the effects panel.
- Update the README's feature, parameter, preset, and MIDI sections to match v3.

## Constraints and verification

- Keep the current JUCE 8.0.9 and C++20 build structure.
- The local verification is a Release plugin build. Do not add or run local tests as part of this task.
- The existing GitHub workflow runs its current tests and plugin validation on macOS and Windows, builds Linux targets, and creates a GitHub Release from a `v*` tag. Monitor that workflow after pushing.
- Update the CMake version and UI version label to 3.0.0, commit with the personal noreply git identity, push `main`, and push tag `v3.0.0`.
- Use issue #1 as the progress tracker and close it after the release workflow succeeds.

## Remaining implementation calls

- The builder chooses exact synthesis equations and anti-aliasing details while preserving the required MIDI fundamental.
- The builder chooses the final parameter ranges and names for transient, tone, noise, slapback, and space controls, keeping the interface readable in its current size.
- The builder chooses the 44 preset names and values, with every sound remaining within the requested trance focus.
