# Commercial feature research: openClapper

## Reference

D16's official PunchBox 2 page describes a layered kick workflow and adjustable pitch, filtering, and modulation for its generator layers. Its official PunchBox manual exposes per-layer high cut, low cut, decay, and pitch controls.

- [PunchBox 2 product page](https://d16.pl/punchbox2)
- [PunchBox user manual](https://d16.pl/pub/manuals/PunchBox-manual-gb.pdf)

This is product research for sound-design patterns. openClapper keeps its own synthesis, names, presets, and DSP.

## Capability gaps and implemented changes

1. **Tonal body pitch sweep.** The body tracked MIDI pitch without a separate sweep. Added host-automatable sweep depth (−24 to +24 semitones) and sweep time controls.
2. **Independent body high/low cut.** Noise had HPF/LPF, but the tonal body did not. Added its own automatable high-pass and low-pass cutoffs, leaving noise filtering independent.
3. **Transient decay.** Clicks used a fixed short envelope. Added host-automatable click decay so the transient can range from a sharp crack to an extended attack layer.

## Verification

- Release AU, VST3, LV2, and Standalone targets built successfully.
- CTest passed: **43/43**.
- Refreshed and inspected assets/openClapper-screenshot.png, assets/openClapper-screenshot-advanced.png, and assets/openClapper-performance.png. Advanced view shows the new body sweep, body filters, and click decay controls.
