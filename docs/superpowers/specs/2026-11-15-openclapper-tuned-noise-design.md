# openClapper Tuned Noise Restoration Design

## Intent

Restore the MIDI-tuned noise resonance that Bandpass patches lost during the v3 synthesis refactor.

## Approaches

- Add noise to every tone mode. This changes SubThump, SaturatedTri, and CyberSquare patch character beyond the regression.
- Restore the former Bandpass exciter, centered on the current MIDI note. This matches the pre-refactor design and keeps the other modes intact; use this approach.
- Revert the full v3 synth. This also removes MIDI pitch tracking and unrelated fixes.

## Design

For `ToneMode::Bandpass`, excite the tuned resonator with the existing noise source and the original short impulse, using the received MIDI fundamental plus the patch's Q, decay, and level. Keep the v3 partial synthesis for the other tone modes. The factory preset bank and its Bandpass selections survived the refactor; restoring the engine route restores their intended sound, so do not rewrite unrelated preset values. Update the plugin version to 3.1.0.

## Success Criteria

- Bandpass output changes with noise type when broadband output is muted, proving the tuned exciter is active.
- MIDI note tuning, all tone modes, and all 44 factory presets remain valid.
- Local format builds and GitHub Actions release complete successfully.
