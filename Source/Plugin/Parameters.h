#pragma once

#include "ClapEngine.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace openclapper::params {

// Pre-Clap Flam (Micro-bursts)
inline constexpr const char* kPreBursts = "pre_bursts";
inline constexpr const char* kPreSpread = "pre_spread";
inline constexpr const char* kPreDecay = "pre_decay";
inline constexpr const char* kPreJitter = "pre_jitter";
inline constexpr const char* kFlamSpread = "flam_spread";
inline constexpr const char* kPreLevel = "pre_level";

// Resonant Body / Tone
inline constexpr const char* kToneMode = "tone_mode";
inline constexpr const char* kToneQ = "tone_q";
inline constexpr const char* kToneDecay = "tone_decay";
inline constexpr const char* kToneLevel = "tone_level";

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

// Reverse & Sync
inline constexpr const char* kReverseMode = "reverse_mode";
inline constexpr const char* kReverseMix = "reverse_mix";

// Global / Master
inline constexpr const char* kStereoWidth = "stereo_width";
inline constexpr const char* kOutputGain = "output_gain";
inline constexpr const char* kPreset = "preset";

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
openclapper::core::EngineSettings readSettings(const juce::AudioProcessorValueTreeState& apvts);

} // namespace openclapper::params
