# openClapper v3.4.0

[Install with OpenVST Manager](https://github.com/alanmarcero/alan-m-OpenVSTManager/releases/latest)

openClapper is a pitched, four-layer clap synthesizer. Play the resonant body from a MIDI keyboard, add a flam of up to five hands, and set how much noise follows the strike. **Audition A4** previews the current sound from any page.

![Clap editor with a rendered waveform marking the main strike, the preset browser, and controls for hand spacing, tuned body, noise tail and output](assets/openClapper-screenshot.png)

## Choose a clap

The 45 factory sounds are arranged in five families of nine. **Previous** and **Next** step through the bank. All factory sounds play as one-shots.

| Family | Try | Sound |
| --- | --- | --- |
| Close | Downbeat, Cupped Palm, Mono Print | Short, focused hands for the backbeat, including a fully mono clap. |
| Flam | Two Hands, Pocket Drag, Five Across | Spaced strikes with different timing, pan, and amounts of humanization. |
| Wide | Balcony, Palm Circle, Opposite Walls | Staggered hands and a wider noise layer for ensemble claps. |
| Gate | Short Plate, Concrete, High Gallery | Early room reflections with 70–125 ms gates. |
| Roll | Sixteenth, Ratchet, Ghost Palm | Compact hits for fast patterns and fills. |

The animated hands beside the page tabs meet on each host beat and follow tempo changes and seeks. They rest when playback stops. Standalone and hosts without transport timing use the internal 138 BPM clock.

The **Clap** page shows a dry A4 preview. The peach section contains the pre-claps. The vertical marker shows when the main strike begins. Change **Bursts** or **Spacing** to see it move. **Body decay** and **Noise decay** shape the pitched body and noise tail separately.

## Shape the layers

On **Layers**, set the timing, decay and stereo placement of the pre-claps. Choose a pitched body voice and adjust its pitch sweep and filters. Noise has its own attack, hold, decay, curve and stereo width. The transient has its own shape, frequency and decay.

In v3.3, each flam contact takes 0.35 ms to rise, and successive contacts build toward the main strike. Pink noise uses independent left and right filters. Lo-fi noise samples the two channels independently on a shared 12 kHz clock, keeping its texture consistent across host sample rates.

![Layers page showing the four sound sources and their individual envelopes and filters](assets/openClapper-screenshot-advanced.png)

## Play as a synthesizer

Open **MIDI & envelope** and select **Hold notes**. The body and noise sustain while the key is down and release when the matching note and channel send note-off. Set attack and decay up to 10 seconds, sustain from 0 to 1, and release up to 20 seconds. Switch to **One-shot** to use the individual layer decays again. The audition button automatically releases held sounds.

Set the inclusive MIDI key and velocity ranges. Choke group 0 allows overlaps. Groups 1–16 stop older voices in the same group. Audition bypasses the key and velocity ranges. Solo or mute each layer independently. Body, noise, and transient have adjustable velocity response from 0 (constant) through 1 (linear) to 2 (squared). Pre-claps follow velocity.

**Roll & fill** repeats a held note at 1/8, 1/16, 1/32, or 1/16 triplet spacing. Set 1–32 hits. Rolls stop at note-off, transport stop, or seek.

![MIDI and envelope page with Hold notes selected, an editable ADSR, trigger zones, roll controls and layer mute, solo and velocity settings](assets/openClapper-performance.png)

## Add effects and route audio

On **Effects**, adjust drive, compression, gated stereo reverb, one-repeat slapback, reverse swell, limiting, stereo width, and output gain. Slapback follows host tempo and uses 138 BPM if the host supplies none. In hosts with multi-output support, route the body, noise, transient, and pre-clap to separate stereo buses. Each bus includes its layer controls and master gain, before shared effects and stereo width.

![Effects page showing drive, compressor, gated room, echo, reverse, and output controls](assets/openClapper-effects.png)

MIDI CC 7 adjusts master level and CC 74 controls noise brightness. CC 120 stops sound and effect tails, CC 121 resets the two controller overrides, and CC 123 releases held notes.

## Formats and compatibility

macOS includes AU, VST3, LV2, and Standalone. Windows and Linux include VST3, LV2, and Standalone. Older sessions restore their saved parameter values and appear as **Custom sound**. The original 44 host program positions and automation ranges are preserved. The editor offers all 45 sounds.

MIT License. See [LICENSE](LICENSE).
