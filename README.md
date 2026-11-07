# openClapper

[Install with OpenVST Manager](https://github.com/alanmarcero/alan-m-OpenVSTManager/releases/latest)

> **Clap on. Clap off.**

EDM & Trance clap drum synthesizer instrument plugin with multi-burst micro-slap flam, 12-note chromatic body tuning, multi-texture noise shaping, impact distortion, studio dynamics with brickwall limiter, trance gated reverb, host-synced reverse swells, WAV session lock-in, and the animated **DJ Clappy** cyber mascot stage.

Packages include **VST3**, **AU**, **LV2**, and a **Standalone** application on macOS, Windows, and Linux.

![openClapper hero](assets/openClapper-hero.png)

---

## Slogan & Concept: *Clap on. Clap off.*

Inspired by the classic sound-activated switch slogan reimagined for electronic dance music festivals and cyberpunk club rigs.

- **DJ Clappy Stage**: Features the charismatic 3D rave robot DJ mascot, **DJ Clappy**, equipped with glowing neon cyan LED visor shades, cyber headphones, and robotic hands.
- **Trigger Animation**: When triggered by MIDI or the stage button, DJ Clappy's hands swing together in sync with the multi-burst pre-clap flam, triggering radiant neon shockwave rings, spotlight beam pulses, and a pumping subwoofer cone.
- **Neon Telemetry**: Visual indicators display **CLAP ON** during the transient strike and sustain, transitioning to **CLAP OFF** when the tail gates or finishes.

---

## Sound Architecture & Knobs

Every sonic dimension of the clap has dedicated hands-on controls:

### 1. Pre-Clap Flam (Micro-bursts)
Recreates the natural physics of human hands or crowd claps arriving slightly out of sync before the collective impact:
- **Bursts (`pre_bursts`)**: 1 to 5 micro-bursts prior to the main slap.
- **Spread (`pre_spread`)**: 2 to 45 ms spacing between micro-bursts.
- **Decay (`pre_decay`)**: 2 to 35 ms decay envelope of each micro-burst.
- **Humanize (`pre_jitter`)**: 0% to 100% timing and micro-pitch jitter per burst for organic feel.
- **Level (`pre_level`)**: Relative volume of the pre-clap bursts.

### 2. Resonant Body & Tonal Core
Injects musical punch, harmonic bite, and chest thump into the clap:
- **Note (`tone_note`) & Octave (`tone_octave`)**: 12-note chromatic root tuning (C through B, Octaves 1–5).
- **Fine (`tone_fine`)**: -50 to +50 cents fine tuning.
- **Freq (`tone_freq`)**: Bandpass center frequency (200 Hz to 4000 Hz).
- **Res / Q (`tone_q`)**: Filter resonance (0.5 to 12.0) from smooth acoustic body to laser tone.
- **Decay (`tone_decay`)**: 15 to 600 ms body ring length.
- **Pitch Drop (`tone_pitch_env`)**: -24 to +24 semitones punch drop depth.
- **Drop Time (`tone_pitch_decay`)**: 3 to 80 ms pitch envelope time.
- **Level (`tone_level`)**: Level of the tuned tonal body.

### 3. Noise & Sizzle Engine
The substance and texture of the clap:
- **Type (`noise_type`)**:
  - `White`: Classic bright, continuous broad-spectrum sizzle.
  - `Pink`: Warm, weighted, natural organic clap noise.
  - `Velvet`: Sparse, crisp Poisson micro-impulses for hyper-modern EDM snap.
  - `Lo-Fi`: Decimated, bit-reduced 8-bit crunchy retro digital noise.
- **Attack (`noise_attack`)**: 0 to 20 ms attack ramp.
- **Hold (`noise_hold`)**: 0 to 30 ms transient hold.
- **Decay (`noise_decay`)**: 20 to 1000 ms decay time.
- **Curve (`noise_curve`)**: -1.0 (steep punchy exponential) to +1.0 (dense logarithmic sustain).
- **HPF (`noise_hpf`)**: 20 Hz to 2500 Hz 12 dB/oct high-pass filter to clean up low-end mud.
- **LPF (`noise_lpf`)**: 800 Hz to 20000 Hz 12 dB/oct low-pass filter to tame harsh highs.
- **Width (`noise_stereo_width`)**: 0.0 (dead center mono) to 2.0 (super-wide Haas decorrelation).
- **Level (`noise_level`)**: Overall noise level.

### 4. Transient Click
- **Click Lvl (`click_level`)**: High-frequency needle transient to poke through dense supersaw stacks.
- **Click Freq (`click_freq`)**: 1000 Hz to 12000 Hz click center frequency.

### 5. Impact Distortion
- **Dist Type (`dist_type`)**:
  - `Tape`: Warm, asymmetric soft tape saturation.
  - `Tube`: 2nd/3rd harmonic triode tube overdrive with rich warmth.
  - `Hard Clip`: Aggressive brickwall hard clipping for festival mainstage impact.
  - `Wavefold`: Sine wavefolding distortion for metallic cybernetic bite.
- **Drive (`dist_drive`)**: 0 to +28 dB saturation drive with automatic level compensation.
- **Tone (`dist_tone`)**: Post-distortion tilt EQ (-1.0 dark to +1.0 bright sizzle).
- **Dist Mix (`dist_mix`)**: Parallel dry/wet saturation blend.

### 6. Studio Dynamics: Compressor & Brickwall Limiter
- **Compressor**:
  - **Threshold (`comp_thresh`)**: -40 dB to 0 dB.
  - **Ratio (`comp_ratio`)**: 1:1 to 20:1.
  - **Attack (`comp_attack`)**: 0.1 to 50 ms peak follower.
  - **Release (`comp_release`)**: 10 to 500 ms smooth release.
  - **Makeup (`comp_makeup`)**: 0 to +24 dB clean gain.
  - **Comp Mix (`comp_mix`)**: Parallel NY-style compression blend.
- **Brickwall Limiter**:
  - **Ceiling (`limiter_ceiling`)**: -12 dB to 0 dB true ceiling.
  - **Release (`limiter_release`)**: 5 to 100 ms release.
  - **Limiter Enabled (`limiter_enabled`)**: Zero-latency lookahead soft-knee brickwall catch ensuring 0.0 dBFS overshoots never occur.

### 7. Trance Gated Reverb
- **Size (`reverb_size`)**: Room diffuser scale.
- **Decay (`reverb_decay`)**: 50 to 2500 ms tail decay.
- **Damping (`reverb_damp`)**: High-frequency air absorption.
- **Gate Time (`reverb_gate`)**: 0 (off / natural tail) to 600 ms instant gate cutoff with smooth 15 ms anti-click release.
- **Rev Mix (`reverb_mix`)**: Reverb wet/dry blend.

### 8. Host-Synced Reverse Swells
- **Rev Sync (`reverse_mode`)**:
  - `Off`: Forward playback only.
  - `Beat End (Sync)`: Swell ends exactly on the next beat boundary.
  - `Beat 4 Auto`: Automatically reverses on Beat 4 of the measure into the downbeat.
  - `1/2 Beat`: Swell timed to the next half beat.
  - `1/4 Beat`: Swell timed to the next quarter beat.
- **Rev Mix (`reverse_mix`)**: Wet balance of reverse swell.

### 9. Master & Performance
- **MIDI Mode (`midi_mode`)**:
  - `Fixed (Root)`: Every incoming MIDI note plays the exact panel tuning.
  - `Note Tracking`: Transposes the tuned pitch relative to incoming MIDI notes.
- **LOCK IN (WAV) (`lock_in`)**: Renders and bakes the complete synthesized clap into an internal 32-bit float RIFF WAV buffer stored inside the DAW session. Guarantees 100% bit-exact recall and zero CPU overhead.
- **Master Width (`stereo_width`)**: Global Mid/Side stereo field control (0 = mono, 1 = normal, 2 = wide).
- **Output Gain (`output_gain`)**: -36 dB to +12 dB master output trim.

---

## 32 EDM / Trance Factory Presets

| # | Preset Name | Character & Style |
|---|---|---|
| 01 | **Init Trance Clap** | Balanced, punchy staple trance clap with pre-bursts and crisp tail. |
| 02 | **Uplifting Euphoria** | Huge soaring uplifting trance clap with wide stereo sizzle and gated reverb. |
| 03 | **Psytrance Laser Snap** | Ultra-tight laser tone punch, fast decay, resonant tone snap. |
| 04 | **Big Room Festival Stomp** | Massive stadium-filling compressed clap with heavy drive and wide body. |
| 05 | **138 BPM Peak Time** | Fast, crisp, cutting transient engineered for high-BPM driving trance. |
| 06 | **Dutch Hardstyle Smacker** | Hard-clipped, heavy tube distortion for hard dance & hardstyle. |
| 07 | **Progressive Velvet Air** | Smooth velvet noise, subtle pre-delay, warm tape saturation for deep progressive. |
| 08 | **Future Rave Cyber Slap** | Metallic wavefold bite, bright click, modern future rave signature. |
| 09 | **90s Eurodance Gated Classic** | Nostalgic 90s gated room clap reminiscent of classic Eurodance anthems. |
| 10 | **Vocal Layering Clap** | Scooped mids, ultra-wide stereo spread, designed to sit behind lead vocals. |
| 11 | **Electro Sizzle Punch** | Gritty lo-fi noise, high drive, aggressive transient attack. |
| 12 | **Tech Trance Acid Bite** | Resonant tone peak, tight gate, hard impact for dark tech trance. |
| 13 | **Hands Up German Party** | High-energy, snappy, bright flam clap with uplifting room reverb. |
| 14 | **Stadium Crowd Swarm** | 5-burst flam simulating thousands of festival clubbers clapping in unison. |
| 15 | **Cyberpunk Neon Slam** | Heavy saturation, wavefolded tone, wide stereo sizzle. |
| 16 | **Anjunabeats Melodic Dream** | Lush, warm, tape-saturated clap with velvet decay and gentle comp. |
| 17 | **Melbourne Bounce Spank** | Punchy mid tone, fast release compressor for bouncy electro groove. |
| 18 | **Vocal Chop Pre-Swell** | Host-synced reverse build into explosive crisp clap. |
| 19 | **Raw Warehouse Techno** | Dark HPF/LPF, tube overdrive, gritty lo-fi noise texture. |
| 20 | **Trance Energy 2000** | Classic early-2000s energetic trance clap with long gated hall. |
| 21 | **Dual Flam Stereo Split** | Wide Haas micro-spread, distinct 2-burst flam for maximum stereo width. |
| 22 | **Hardcore Punch In Face** | Maximum limiter smash, hard-clip distortion, ultra-compressed impact. |
| 23 | **Sunset Balearic Chill** | Soft attack, pink noise warmth, gentle acoustic-style tone. |
| 24 | **Goa Psy Resonance Ring** | High-Q tone body ringing in key, psychedelic sizzle. |
| 25 | **Mainstage Drop Impact** | Layered click spike, massive compressor makeup, festival-ready loudness. |
| 26 | **Minimal Click & Flam** | Sparse, dry, surgical transient with zero tail. |
| 27 | **Dirty Dutch Pitch Drop** | Exaggerated pitch dive on the body tone for squeaky bounce. |
| 28 | **Gated Snare-Clap Hybrid** | Tuned 200 Hz body thump layered with wide noise sizzle. |
| 29 | **Euphoric Whiteout Noise** | Massive white noise burst with high-pass sweep and shimmering tail. |
| 30 | **Cyber Clapper Signature** | The signature mascot sound: punchy tone, crisp 3-burst flam, tape warmth, gated space. |
| 31 | **Fast Roll 16th Anti-Mud** | Short decay, tight envelope, optimized for rapid 16th/32nd-note clap rolls. |
| 32 | **Beat 4 Reverse Pickup** | Auto-reversing on Beat 4 into the downbeat clap. |

---

## MIDI Implementation

- **Omni reception**: All channels are read.
- **One-Shot Voice**: Note-off leaves the clap ringing until its envelope finishes naturally or `All Sound Off` arrives.
- **Velocity**: Linearly scales synthesis amplitude and locked WAV playback.
- **MIDI Controllers**:
  - `CC 7` (Volume): Master output volume trim.
  - `CC 74` (Brightness): Real-time noise low-pass cutoff filter sweep.
  - `CC 120` / `123` (All Sound Off / All Notes Off): Silences active voices immediately.
  - `CC 121` (Reset All Controllers): Resets CC 7 and CC 74 to default 127.

---

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

---

## Repository Layout

- `Source/Core`: Pure C++20 DSP engine with zero allocations on the audio thread (`ClapSynth`, `Effects`, `ClapEngine`).
- `Source/Plugin`: JUCE 8 plugin implementation, custom `EdmLookAndFeel`, `Controls`, `Panels`, `Parameters`, `Presets`, and `PluginEditor`.
- `Tests`: Catch2 unit tests covering synthesis, filters, distortion, compression, brickwall limiting, WAV lock-in, and plugin parameters.
- `assets`: Embedded high-resolution artwork (`dj-clappy.png`, `openclapper-faceplate.png`, `openClapper-hero.png`).
- `.github/workflows/build.yml`: Multi-platform matrix CI/CD building macOS universal (arm64 & x86_64), Windows x64, Linux, running pluginval and packaging release archives.

---

## License

MIT License. Copyright (c) 2026 Alan Marcero.
JUCE is dual-licensed under AGPL-3.0 or commercial license.
