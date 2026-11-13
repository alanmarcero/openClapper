# openClapper v3

[Install with OpenVST Manager](https://github.com/alanmarcero/alan-m-OpenVSTManager/releases/latest)

Clap on. Clap off.

openClapper is a trance-focused clap synthesizer for VST3, AU, LV2, and Standalone on macOS, Windows, and Linux. Version 3 replaces pitch selection and sample lock-in with direct MIDI tuning: each incoming note sets the clap's tonal fundamental. MIDI note 69 is A4 at 440 Hz, so playing A4 produces an A-tuned clap; every semitone and octave follows equal temperament.

The synthesis engine builds each hit from a tuned resonant body, layered noise, a transient click, and up to five pre-clap bursts. Eight voices can overlap. Shared effects add host-tempo slapback, gated stereo reverb, distortion, compression, limiting, and stereo width.

![openClapper user interface](assets/openClapper-screenshot.png)

## MIDI tuning

Incoming MIDI note-ons trigger the corresponding pitch directly, on any MIDI channel. The tonal body follows the standard MIDI frequency mapping:

```text
frequency = 440 × 2^((MIDI note - 69) / 12)
```

For example, notes 57, 69, and 81 produce A3 at 220 Hz, A4 at 440 Hz, and A5 at 880 Hz. Velocity scales the voice level. The on-screen audition button triggers A4.

## Sound controls

| Section | Controls | What they shape |
|---|---|---|
| Pre-clap flam | Bursts, spread, decay, humanize, pan, level | Micro-bursts and stereo movement before the main hit |
| Resonant body | Mode, Q, decay, level | MIDI-tuned body character and sustain |
| Noise | Type, attack, hold, decay, curve, HPF, LPF, width, level | The clap's broadband texture and stereo spread |
| Transient click | Type, level, frequency | The initial needle, crack, or thump |
| Distortion | Type, drive, tone, mix | Tape, tube, hard-clip, or wavefold color |
| Compressor | Threshold, ratio, attack, release, makeup, mix | Transient density and loudness |
| Limiter | Ceiling, release, enabled | Output peak control |
| Gated reverb | Size, decay, damping, gate time, mix | Shared stereo room and gated tail |
| Slapback | Division, mix | One host-synced repeat with no feedback |
| Reverse swell | Sync mode, mix | Host-tempo reverse build into the clap |
| Master | Width, output gain | Final stereo image and level |

### Host-synced slapback

Slapback uses the tempo reported by the host and keeps its delay tail between hits. Choose Off, 1/16, dotted 1/16, 1/8, dotted 1/8, or 1/4. The delay makes one repeat without feedback. If the host has no usable tempo, the engine uses 138 BPM.

## Factory presets

The 44 factory presets are redesigned for trance, from tight gated impacts to wide euphoric and psytrance textures. Each preset recalls the full sound and effect settings, including slapback Off when that preset does not use a repeat.

| # | Preset | # | Preset |
|---:|---|---:|---|
| 01 | Mainstage Anthem Gate | 23 | Full-On Bloom |
| 02 | Uplift Wide Gate | 24 | Forest Crackle |
| 03 | Euphoric Bloom 1/8 Slap | 25 | Night Psy Metal |
| 04 | Festival Stack | 26 | Dark Psy Pulse |
| 05 | 138 Peak Punch | 27 | Tech Trance Stab |
| 06 | Dutch Hard Gate | 28 | Rave Siren Pickup |
| 07 | Progressive Velvet Air | 29 | Millennium Hands Up |
| 08 | Future Rave Cyber Slap | 30 | Classic 138 Gate |
| 09 | Euro Gated Classic | 31 | Dutch Lead Impact |
| 10 | Hands Up Shimmer | 32 | Reverse Skyline |
| 11 | Progressive Glass | 33 | Big Room Lift |
| 12 | Sunset Wide | 34 | Piano Anthem Gate |
| 13 | Rolling Progressive | 35 | Laser Stadium |
| 14 | Deep Pool Clap | 36 | Vintage Euro Lift |
| 15 | Late Night Pulse | 37 | Minimal Club Tap |
| 16 | Aurora Whisper | 38 | Warm Dub 1/8 |
| 17 | Supersaw Haze | 39 | Dry Utility Clap |
| 18 | Progressive Dotted Slap | 40 | Wide Utility Clap |
| 19 | Psy Fractal Snap | 41 | Tight Roll Clap |
| 20 | Offbeat Goa | 42 | Quarter Note Bloom |
| 21 | Spiral Acid Bite | 43 | MIDI Reference Clap |
| 22 | Cosmic Ping 1/4 | 44 | Euphoric Trance Lift |

## MIDI controllers

| Controller | Function |
|---|---|
| CC 7 | Master output volume |
| CC 74 | Noise low-pass cutoff |
| CC 120 / 123 | All sound off / all notes off |
| CC 121 | Reset CC 7 and CC 74 |

## Build

Building requires CMake 3.22 or newer and a C++20 compiler. JUCE 8.0.9 and Catch2 are fetched by CMake.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

## Repository layout

| Path | Contents |
|---|---|
| `Source/Core` | C++20 clap synthesis, voice engine, slapback, and effects |
| `Source/Plugin` | JUCE plugin wrapper, interface, controls, parameters, and presets |
| `Tests` | Catch2 unit tests |
| `assets` | Faceplate graphics and UI screenshots |
| `.github/workflows` | Multi-platform CI and release builds |

## License

MIT License. Copyright (c) 2026 Alan Marcero.
JUCE is dual-licensed under AGPL-3.0 or commercial license.
