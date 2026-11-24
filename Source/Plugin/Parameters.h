#pragma once

#include "ClapEngine.h"
#include "../suite/EqNotes.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <utility>

namespace openclapper::params {

// Pre-Clap Flam (Micro-bursts)
inline constexpr const char* kPreBursts = "pre_bursts";
inline constexpr const char* kPreSpread = "pre_spread";
inline constexpr const char* kPreDecay = "pre_decay";
inline constexpr const char* kPreJitter = "pre_jitter";
inline constexpr const char* kFlamSpread = "flam_spread";
inline constexpr const char* kPreLevel = "pre_level";

// Resonant Body / Tone
inline constexpr const char* kToneQ = "tone_q";
inline constexpr const char* kToneDecay = "tone_decay";
inline constexpr const char* kTonePitchEnv = "tone_pitch_env";
inline constexpr const char* kTonePitchDecay = "tone_pitch_decay";
inline constexpr const char* kToneHPF = "tone_hpf";
inline constexpr const char* kToneLPF = "tone_lpf";
inline constexpr const char* kToneLevel = "tone_level";
inline constexpr const char* kImpactPunch = "impact_punch";

// Noise & Sizzle
inline constexpr const char* kNoiseType = "noise_type";
inline constexpr const char* kNoiseAttack = "noise_attack";
inline constexpr const char* kNoiseHold = "noise_hold";
inline constexpr const char* kNoiseDecay = "noise_decay";
inline constexpr const char* kNoiseCurve = "noise_curve";
inline constexpr const char* kNoiseHPF = "noise_hpf";
inline constexpr const char* kNoiseLPF = "noise_lpf";
inline constexpr const char* kNoiseStereoWidth = "noise_stereo_width";
inline constexpr const char* kNoiseLevel = "noise_level";

// Click / Transient
inline constexpr const char* kClickType = "click_type";
inline constexpr const char* kClickLevel = "click_level";
inline constexpr const char* kClickFreq = "click_freq";
inline constexpr const char* kClickDecay = "click_decay";

// Impact Distortion
inline constexpr const char* kDistType = "dist_type";
inline constexpr const char* kDistDrive = "dist_drive";
inline constexpr const char* kDistTone = "dist_tone";
inline constexpr const char* kDistMix = "dist_mix";

// Dynamics: Compressor
inline constexpr const char* kCompThresh = "comp_thresh";
inline constexpr const char* kCompRatio = "comp_ratio";
inline constexpr const char* kCompAttack = "comp_attack";
inline constexpr const char* kCompRelease = "comp_release";
inline constexpr const char* kCompMakeup = "comp_makeup";
inline constexpr const char* kCompMix = "comp_mix";

// Dynamics: Limiter
inline constexpr const char* kLimiterCeiling = "limiter_ceiling";
inline constexpr const char* kLimiterRelease = "limiter_release";
inline constexpr const char* kLimiterEnabled = "limiter_enabled";

// Trance Gated Reverb
inline constexpr const char* kReverbSize = "reverb_size";
inline constexpr const char* kReverbDecay = "reverb_decay";
inline constexpr const char* kReverbDamp = "reverb_damp";
inline constexpr const char* kReverbGate = "reverb_gate";
inline constexpr const char* kReverbMix = "reverb_mix";

// Host-Synced Slapback
inline constexpr const char* kSlapbackDivision = "slapback_division";
inline constexpr const char* kSlapbackMix = "slapback_mix";
inline constexpr const char* kSlapbackFeedback = "slapback_feedback";
inline constexpr const char* kClapDoubler = "clap_doubler";
inline constexpr const char* kPitchHumanize = "pitch_humanize";

// Reverse & Sync
inline constexpr const char* kReverseMode = "reverse_mode";
inline constexpr const char* kReverseMix = "reverse_mix";

// Global / Master & Output EQ
inline constexpr const char* kStereoWidth = "stereo_width";
inline constexpr const char* kOutputGain = "output_gain";
inline constexpr const char* kEqLowFreq = "eq_low_freq";
inline constexpr const char* kEqLowDb = "eq_low_db";
inline constexpr const char* kEqMidFreq = "eq_mid_freq";
inline constexpr const char* kEqMidDb = "eq_mid_db";
inline constexpr const char* kEqHighFreq = "eq_high_freq";
inline constexpr const char* kEqHighDb = "eq_high_db";
inline constexpr const char* kPreset = "preset";
// Automatable Preset Index range kept for older sessions. The host program list is the full factory bank.
inline constexpr int kHostProgramCount = 44;

inline constexpr std::array<const char*, 4> kLayerNames{"Body", "Noise", "Transient", "Pre-clap"};
inline constexpr std::array<const char*, 4> kLayerMute{"body_mute", "noise_mute", "click_mute", "pre_mute"};
inline constexpr std::array<const char*, 4> kLayerSolo{"body_solo", "noise_solo", "click_solo", "pre_solo"};
inline constexpr std::array<const char*, 3> kLayerVelocity{"body_velocity", "noise_velocity", "click_velocity"};

struct PerformanceParameter {
    const char* id;
    const char* name;
    float minimum, maximum, initial;
};
inline constexpr std::array<PerformanceParameter, 11> kPerformanceParameters{{
    {"key_low", "MIDI Key Low", 0, 127, 0}, {"key_high", "MIDI Key High", 0, 127, 127},
    {"velocity_low", "Velocity Low", 1, 127, 1}, {"velocity_high", "Velocity High", 1, 127, 127},
    {"choke_group", "Choke Group (0 Off)", 0, 16, 0}, {"roll_hits", "Roll / Fill Hits", 1, 32, 4},
    {"amp_attack", "Amp Attack (ms)", 0, 10000, 0}, {"amp_decay", "Amp Decay (ms)", 0, 10000, 100},
    {"amp_sustain", "Amp Sustain", 0, 1, 0.8f}, {"amp_release", "Amp Release (ms)", 0, 20000, 200},
    {"amp_enabled", "Full ADSR", 0, 1, 0}
}};

// One decimal, "+" on boosts and never "-0.0", as in "+3.0 dB".
juce::String formatSigned(float value, const juce::String& unit);
// "4.5 ms" below 10 ms, "160 ms" below 1 s, then "1.20 s".
juce::String formatMilliseconds(float ms);

inline constexpr int kEqLowMinMidi = suite::eq::kLowMinMidi;
inline constexpr int kEqLowMaxMidi = suite::eq::kLowMaxMidi;
inline constexpr int kEqMidMinMidi = suite::eq::kMidMinMidi;
inline constexpr int kEqMidMaxMidi = suite::eq::kMidMaxMidi;
inline constexpr int kEqHighMinMidi = suite::eq::kHighMinMidi;
inline constexpr int kEqHighMaxMidi = suite::eq::kHighMaxMidi;
inline constexpr const char* kKeyTracking = "key_tracking";

[[nodiscard]] std::pair<int, int> eqBandMidiLimits(const juce::String& freqParamId) noexcept;

float eqNoteToHz(int midiNote) noexcept;
int eqHzToNearestMidi(float hz, int minMidi, int maxMidi) noexcept;
juce::String formatEqNote(float hz, int minMidi, int maxMidi);
float parseEqNoteOrHz(const juce::String& text, int minMidi, int maxMidi);

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
openclapper::core::EngineSettings readSettings(const juce::AudioProcessorValueTreeState& apvts);

} // namespace openclapper::params
