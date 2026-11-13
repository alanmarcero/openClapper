#include "Parameters.h"
#include <vector>

namespace openclapper::params {

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Pre-Clap Flam
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{ kPreBursts, 1 }, "Pre-Clap Bursts", 1, 5, 3));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kPreSpread, 1 }, "Pre-Clap Spread", juce::NormalisableRange<float>(2.0f, 45.0f, 0.1f), 12.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kPreDecay, 1 }, "Pre-Clap Decay", juce::NormalisableRange<float>(2.0f, 35.0f, 0.1f), 8.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kPreJitter, 1 }, "Pre-Clap Humanize", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.15f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kFlamSpread, 1 }, "Flam Pan", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.45f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kPreLevel, 1 }, "Pre-Clap Level", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.85f));

    // Resonant Body / Tone
    juce::StringArray toneModes{ "Bandpass", "Sub Thump", "Saturated Tri", "Cyber Square" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ kToneMode, 1 }, "Tone Mode", toneModes, 0));
    juce::StringArray noteNames{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ kToneNote, 1 }, "Tone Note", noteNames, 9)); // A
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{ kToneOctave, 1 }, "Tone Octave", 1, 8, 3));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kToneFine, 1 }, "Tone Fine", juce::NormalisableRange<float>(-50.0f, 50.0f, 0.5f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("ct")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kToneFreq, 1 }, "Tone Freq", juce::NormalisableRange<float>(100.0f, 6000.0f, 1.0f, 0.35f), 1100.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kToneQ, 1 }, "Tone Q", juce::NormalisableRange<float>(0.5f, 12.0f, 0.1f), 3.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kToneDecay, 1 }, "Tone Decay", juce::NormalisableRange<float>(15.0f, 600.0f, 1.0f, 0.35f), 120.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kTonePitchEnv, 1 }, "Tone Pitch Drop", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.5f), 7.0f,
        juce::AudioParameterFloatAttributes().withLabel("st")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kTonePitchDecay, 1 }, "Tone Pitch Decay", juce::NormalisableRange<float>(3.0f, 80.0f, 0.5f), 18.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kToneLevel, 1 }, "Tone Level", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.75f));

    // Noise & Sizzle
    juce::StringArray noiseTypes{ "White", "Pink", "Velvet", "Lo-Fi" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ kNoiseType, 1 }, "Noise Type", noiseTypes, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseAttack, 1 }, "Noise Attack", juce::NormalisableRange<float>(0.0f, 20.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseHold, 1 }, "Noise Hold", juce::NormalisableRange<float>(0.0f, 30.0f, 0.1f), 2.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseDecay, 1 }, "Noise Decay", juce::NormalisableRange<float>(20.0f, 1000.0f, 1.0f, 0.35f), 180.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseCurve, 1 }, "Noise Curve", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), -0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseHPF, 1 }, "Noise HPF", juce::NormalisableRange<float>(20.0f, 2500.0f, 1.0f, 0.35f), 350.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseLPF, 1 }, "Noise LPF", juce::NormalisableRange<float>(800.0f, 20000.0f, 1.0f, 0.35f), 9500.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseStereoWidth, 1 }, "Noise Width", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseLevel, 1 }, "Noise Level", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.85f));

    // Click / Transient
    juce::StringArray clickTypes{ "Needle", "Crack", "Thump" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ kClickType, 1 }, "Click Type", clickTypes, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kClickLevel, 1 }, "Click Level", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kClickFreq, 1 }, "Click Freq", juce::NormalisableRange<float>(1000.0f, 12000.0f, 10.0f, 0.35f), 4500.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    // Impact Distortion
    juce::StringArray distTypes{ "Tape", "Tube", "Hard Clip", "Wavefold" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ kDistType, 1 }, "Distortion Type", distTypes, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kDistDrive, 1 }, "Distortion Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.25f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kDistTone, 1 }, "Distortion Tone", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kDistMix, 1 }, "Distortion Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.35f));

    // Compressor
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kCompThresh, 1 }, "Comp Threshold", juce::NormalisableRange<float>(-40.0f, 0.0f, 0.5f), -14.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kCompRatio, 1 }, "Comp Ratio", juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f), 4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kCompAttack, 1 }, "Comp Attack", juce::NormalisableRange<float>(0.1f, 50.0f, 0.1f, 0.35f), 5.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kCompRelease, 1 }, "Comp Release", juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f, 0.35f), 80.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kCompMakeup, 1 }, "Comp Makeup", juce::NormalisableRange<float>(0.0f, 24.0f, 0.5f), 3.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kCompMix, 1 }, "Comp Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));

    // Limiter
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kLimiterCeiling, 1 }, "Limiter Ceiling", juce::NormalisableRange<float>(-12.0f, 0.0f, 0.1f), -0.3f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kLimiterRelease, 1 }, "Limiter Release", juce::NormalisableRange<float>(5.0f, 100.0f, 0.5f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ kLimiterEnabled, 1 }, "Limiter Enabled", true));

    // Trance Gated Reverb
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kReverbSize, 1 }, "Reverb Size", juce::NormalisableRange<float>(0.1f, 1.0f, 0.01f), 0.65f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kReverbDecay, 1 }, "Reverb Decay", juce::NormalisableRange<float>(50.0f, 2500.0f, 10.0f, 0.35f), 450.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kReverbDamp, 1 }, "Reverb Damp", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kReverbGate, 1 }, "Reverb Gate", juce::NormalisableRange<float>(0.0f, 600.0f, 5.0f), 160.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kReverbMix, 1 }, "Reverb Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.35f));

    juce::StringArray slapbackDivisions{ "Off", "1/16", "Dotted 1/16", "1/8", "Dotted 1/8", "1/4" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ kSlapbackDivision, 1 }, "Slapback Division", slapbackDivisions, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kSlapbackMix, 1 }, "Slapback Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f));

    // Reverse & Sync
    juce::StringArray revModes{ "Off", "Beat End (Sync)", "Beat 4 Auto", "1/2 Beat", "1/4 Beat" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ kReverseMode, 1 }, "Reverse Mode", revModes, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kReverseMix, 1 }, "Reverse Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    // Global / Master
    juce::StringArray midiModes{ "Fixed (Root)", "Note Tracking" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ kMidiMode, 1 }, "MIDI Mode", midiModes, 0));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ kLockPitch, 1 }, "Lock Pitch", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ kLockIn, 1 }, "Lock In (WAV)", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kStereoWidth, 1 }, "Master Width", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kOutputGain, 1 }, "Output Gain", juce::NormalisableRange<float>(-36.0f, 12.0f, 0.5f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{ kPreset, 1 }, "Preset Index", 0, 31, 0));

    return { params.begin(), params.end() };
}

openclapper::core::EngineSettings readSettings(const juce::AudioProcessorValueTreeState& apvts) {
    openclapper::core::EngineSettings s;

    // Pre-Clap Flam
    s.synth.preClapCount = static_cast<int>(*apvts.getRawParameterValue(kPreBursts));
    s.synth.preClapSpreadMs = *apvts.getRawParameterValue(kPreSpread);
    s.synth.preClapDecayMs = *apvts.getRawParameterValue(kPreDecay);
    s.synth.preClapJitter = *apvts.getRawParameterValue(kPreJitter);
    if (const auto* pFlamSpread = apvts.getRawParameterValue(kFlamSpread)) {
        s.synth.preClapFlamSpread = *pFlamSpread;
    }
    s.synth.preClapLevel = *apvts.getRawParameterValue(kPreLevel);

    // Resonant Body / Tone
    if (const auto* pToneMode = apvts.getRawParameterValue(kToneMode)) {
        s.synth.toneMode = static_cast<openclapper::core::ToneMode>(static_cast<int>(*pToneMode));
    }
    s.synth.note = static_cast<int>(*apvts.getRawParameterValue(kToneNote));
    s.synth.octave = static_cast<int>(*apvts.getRawParameterValue(kToneOctave));
    s.synth.fineCents = *apvts.getRawParameterValue(kToneFine);
    s.synth.toneFreqHz = *apvts.getRawParameterValue(kToneFreq);
    s.synth.toneQ = *apvts.getRawParameterValue(kToneQ);
    s.synth.toneDecayMs = *apvts.getRawParameterValue(kToneDecay);
    s.synth.tonePitchEnv = *apvts.getRawParameterValue(kTonePitchEnv);
    s.synth.tonePitchDecayMs = *apvts.getRawParameterValue(kTonePitchDecay);
    s.synth.toneLevel = *apvts.getRawParameterValue(kToneLevel);

    // Noise & Sizzle
    s.synth.noiseType = static_cast<openclapper::core::NoiseType>(static_cast<int>(*apvts.getRawParameterValue(kNoiseType)));
    s.synth.noiseAttackMs = *apvts.getRawParameterValue(kNoiseAttack);
    s.synth.noiseHoldMs = *apvts.getRawParameterValue(kNoiseHold);
    s.synth.noiseDecayMs = *apvts.getRawParameterValue(kNoiseDecay);
    s.synth.noiseCurve = *apvts.getRawParameterValue(kNoiseCurve);
    s.synth.noiseHPF = *apvts.getRawParameterValue(kNoiseHPF);
    s.synth.noiseLPF = *apvts.getRawParameterValue(kNoiseLPF);
    s.synth.noiseStereoWidth = *apvts.getRawParameterValue(kNoiseStereoWidth);
    s.synth.noiseLevel = *apvts.getRawParameterValue(kNoiseLevel);

    // Click
    if (const auto* pClickType = apvts.getRawParameterValue(kClickType)) {
        s.synth.clickType = static_cast<openclapper::core::ClickType>(static_cast<int>(*pClickType));
    }
    s.synth.clickLevel = *apvts.getRawParameterValue(kClickLevel);
    s.synth.clickFreqHz = *apvts.getRawParameterValue(kClickFreq);

    // Effects
    s.effects.distType = static_cast<openclapper::core::DistortionType>(static_cast<int>(*apvts.getRawParameterValue(kDistType)));
    s.effects.distDrive = *apvts.getRawParameterValue(kDistDrive);
    s.effects.distTone = *apvts.getRawParameterValue(kDistTone);
    s.effects.distMix = *apvts.getRawParameterValue(kDistMix);

    s.effects.compressor.thresholdDb = *apvts.getRawParameterValue(kCompThresh);
    s.effects.compressor.ratio = *apvts.getRawParameterValue(kCompRatio);
    s.effects.compressor.attackMs = *apvts.getRawParameterValue(kCompAttack);
    s.effects.compressor.releaseMs = *apvts.getRawParameterValue(kCompRelease);
    s.effects.compressor.makeupDb = *apvts.getRawParameterValue(kCompMakeup);
    s.effects.compressor.mix = *apvts.getRawParameterValue(kCompMix);

    s.effects.limiter.ceilingDb = *apvts.getRawParameterValue(kLimiterCeiling);
    s.effects.limiter.releaseMs = *apvts.getRawParameterValue(kLimiterRelease);
    s.effects.limiter.enabled = *apvts.getRawParameterValue(kLimiterEnabled) > 0.5f;

    s.effects.slapbackDivision = static_cast<openclapper::core::SlapbackDivision>(
        static_cast<int>(*apvts.getRawParameterValue(kSlapbackDivision)));
    s.effects.slapbackMix = *apvts.getRawParameterValue(kSlapbackMix);

    s.effects.reverbSize = *apvts.getRawParameterValue(kReverbSize);
    s.effects.reverbDecayMs = *apvts.getRawParameterValue(kReverbDecay);
    s.effects.reverbDamp = *apvts.getRawParameterValue(kReverbDamp);
    s.effects.reverbGateMs = *apvts.getRawParameterValue(kReverbGate);
    s.effects.reverbMix = *apvts.getRawParameterValue(kReverbMix);

    s.effects.reverseMode = static_cast<openclapper::core::ReverseSyncMode>(static_cast<int>(*apvts.getRawParameterValue(kReverseMode)));
    s.effects.reverseMix = *apvts.getRawParameterValue(kReverseMix);

    s.effects.stereoWidth = *apvts.getRawParameterValue(kStereoWidth);
    s.effects.outputGainDb = *apvts.getRawParameterValue(kOutputGain);

    s.noteTracking = *apvts.getRawParameterValue(kMidiMode) > 0.5f;
    s.lockIn = *apvts.getRawParameterValue(kLockIn) > 0.5f;

    return s;
}

} // namespace openclapper::params
