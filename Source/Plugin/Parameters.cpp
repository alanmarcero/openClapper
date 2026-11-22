#include "Parameters.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace openclapper::params {

juce::String formatSigned(float value, const juce::String& unit) {
    const float tenths = std::round(value * 10.0f) / 10.0f;
    if (std::abs(tenths) < 0.05f) return "0.0 " + unit;
    return (tenths > 0.0f ? "+" : "") + juce::String(tenths, 1) + " " + unit;
}

juce::String formatMilliseconds(float ms) {
    if (ms < 9.95f) return juce::String(ms, 1) + " ms";
    if (ms < 999.5f) return juce::String(juce::roundToInt(ms)) + " ms";
    return juce::String(ms / 1000.0f, ms < 9995.0f ? 2 : 1) + " s";
}

float eqNoteToHz(int midiNote) noexcept {
    const int clamped = std::clamp(midiNote, 0, 144);
    return 440.0f * std::exp2((static_cast<float>(clamped) - 69.0f) / 12.0f);
}

int eqHzToNearestMidi(float hz, int minMidi, int maxMidi) noexcept {
    if (!std::isfinite(hz) || hz <= 0.0f) return minMidi;
    const int midi = juce::roundToInt(69.0f + 12.0f * std::log2(hz / 440.0f));
    return std::clamp(midi, minMidi, maxMidi);
}

juce::String formatEqNote(float hz, int minMidi, int maxMidi) {
    static constexpr std::array<const char*, 12> kNames{
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    const int midi = eqHzToNearestMidi(hz, minMidi, maxMidi);
    const int octave = midi / 12 - 1;
    return juce::String(kNames[static_cast<size_t>(midi % 12)]) + juce::String(octave);
}

float parseEqNoteOrHz(const juce::String& text, int minMidi, int maxMidi) {
    const auto trimmed = text.trim();
    if (trimmed.isEmpty()) return eqNoteToHz(minMidi);
    const int semitone = juce::String("C D EF G A B").indexOfIgnoreCase(trimmed.substring(0, 1));
    if (semitone >= 0) {
        const auto afterLetter = trimmed.substring(1);
        const int sharp = afterLetter.startsWithChar('#') ? 1 : 0;
        const int flat = afterLetter.startsWithChar('b') ? 1 : 0;
        const auto octaveText = afterLetter.substring(sharp + flat).trim();
        if (octaveText.isNotEmpty() && octaveText.containsOnly("-0123456789")) {
            const int midi = (octaveText.getIntValue() + 1) * 12 + semitone + sharp - flat;
            return eqNoteToHz(std::clamp(midi, minMidi, maxMidi));
        }
    }
    const float number = trimmed.getFloatValue();
    const float hz = trimmed.containsIgnoreCase("k") ? number * 1000.0f : number;
    return eqNoteToHz(eqHzToNearestMidi(hz, minMidi, maxMidi));
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    using Attributes = juce::AudioParameterFloatAttributes;
    // dB, st, ct and ratio text use the JUCE parser, which reads the leading number.
    const auto percent = Attributes()
        .withStringFromValueFunction([](float value, int) { return juce::String(juce::roundToInt(value * 100.0f)) + "%"; })
        .withValueFromStringFunction([](const juce::String& text) { return text.getFloatValue() / 100.0f; });
    const auto signedPercent = Attributes()
        .withStringFromValueFunction([](float value, int) {
            const int rounded = juce::roundToInt(value * 100.0f);
            return (rounded > 0 ? "+" : "") + juce::String(rounded) + "%";
        })
        .withValueFromStringFunction([](const juce::String& text) { return text.getFloatValue() / 100.0f; });
    const auto decibels = Attributes().withStringFromValueFunction([](float db, int) { return formatSigned(db, "dB"); });
    const auto hertz = Attributes()
        .withStringFromValueFunction([](float hz, int) {
            if (juce::roundToInt(hz) < 1000) return juce::String(juce::roundToInt(hz)) + " Hz";
            return juce::String(hz / 1000.0f, hz < 9995.0f ? 2 : 1) + " kHz";
        })
        .withValueFromStringFunction([](const juce::String& text) {
            const bool kilohertz = text.containsIgnoreCase("k");
            return text.getFloatValue() * (kilohertz ? 1000.0f : 1.0f);
        });
    const auto milliseconds = Attributes()
        .withStringFromValueFunction([](float ms, int) { return formatMilliseconds(ms); })
        .withValueFromStringFunction([](const juce::String& text) {
            const auto unit = text.trim().toLowerCase();
            const bool seconds = unit.endsWith("s") && !unit.endsWith("ms");
            return text.getFloatValue() * (seconds ? 1000.0f : 1.0f);
        });

    // Pre-Clap Flam
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{ kPreBursts, 1 }, "Pre-Clap Bursts", 1, 5, 3));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kPreSpread, 1 }, "Pre-Clap Spread", juce::NormalisableRange<float>(0.0f, openclapper::core::kMaxTimingOffsetMs, 0.1f), 1.5f,
        milliseconds));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kPreDecay, 1 }, "Pre-Clap Decay", juce::NormalisableRange<float>(2.0f, 35.0f, 0.1f), 8.0f,
        milliseconds));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kPreJitter, 1 }, "Pre-Clap Humanize", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.15f, percent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kFlamSpread, 1 }, "Flam Pan", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.45f, percent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kPreLevel, 1 }, "Pre-Clap Level", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.85f, percent));

    // Resonant Body / Tone & Impact Slam
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kToneQ, 1 }, "Tone Q", juce::NormalisableRange<float>(0.5f, 12.0f, 0.1f), 3.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kToneDecay, 1 }, "Tone Decay", juce::NormalisableRange<float>(15.0f, 1200.0f, 1.0f, 0.38f), 160.0f,
        milliseconds));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kTonePitchEnv, 1 }, "Body Pitch Sweep", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f,
        Attributes().withStringFromValueFunction([](float value, int) { return formatSigned(value, "st"); })));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kTonePitchDecay, 1 }, "Body Pitch Time", juce::NormalisableRange<float>(3.0f, 600.0f, 0.1f, 0.35f), 28.0f,
        milliseconds));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kToneHPF, 1 }, "Body HPF", juce::NormalisableRange<float>(20.0f, 12000.0f, 1.0f, 0.35f), 20.0f,
        hertz));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kToneLPF, 1 }, "Body LPF", juce::NormalisableRange<float>(100.0f, 20000.0f, 1.0f, 0.35f), 20000.0f,
        hertz));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kToneLevel, 1 }, "Tone Level", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.75f, percent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kImpactPunch, 1 }, "Impact", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.45f, percent));

    // Noise & Sizzle
    juce::StringArray noiseTypes{ "White", "Pink", "Velvet", "Lo-Fi" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ kNoiseType, 1 }, "Noise Type", noiseTypes, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseAttack, 1 }, "Noise Attack", juce::NormalisableRange<float>(0.0f, openclapper::core::kMaxTimingOffsetMs, 0.1f), 0.0f,
        milliseconds));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseHold, 1 }, "Noise Hold", juce::NormalisableRange<float>(0.0f, 90.0f, 0.1f), 12.0f,
        milliseconds));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseDecay, 1 }, "Noise Decay", juce::NormalisableRange<float>(20.0f, 2200.0f, 1.0f, 0.38f), 320.0f,
        milliseconds));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseCurve, 1 }, "Noise Curve", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), -0.25f, signedPercent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseHPF, 1 }, "Noise HPF", juce::NormalisableRange<float>(20.0f, 2500.0f, 1.0f, 0.35f), 350.0f,
        hertz));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseLPF, 1 }, "Noise LPF", juce::NormalisableRange<float>(800.0f, 20000.0f, 1.0f, 0.35f), 9500.0f,
        hertz));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseStereoWidth, 1 }, "Noise Width", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f, percent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kNoiseLevel, 1 }, "Noise Level", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.85f, percent));

    // Click / Transient
    juce::StringArray clickTypes{ "Needle", "Crack", "Thump" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ kClickType, 1 }, "Click Type", clickTypes, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kClickLevel, 1 }, "Click Level", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.4f, percent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kClickFreq, 1 }, "Click Freq", juce::NormalisableRange<float>(1000.0f, 12000.0f, 10.0f, 0.35f), 4500.0f,
        hertz));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kClickDecay, 1 }, "Click Decay", juce::NormalisableRange<float>(0.2f, 20.0f, 0.1f, 0.35f), 0.8f,
        milliseconds));

    // Impact Distortion
    juce::StringArray distTypes{ "Tape", "Tube", "Hard Clip", "Wavefold" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ kDistType, 1 }, "Distortion Type", distTypes, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kDistDrive, 1 }, "Distortion Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.25f, percent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kDistTone, 1 }, "Distortion Tone", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f, signedPercent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kDistMix, 1 }, "Distortion Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.35f, percent));

    // Compressor
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kCompThresh, 1 }, "Comp Threshold", juce::NormalisableRange<float>(-40.0f, 0.0f, 0.5f), -14.0f,
        decibels));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kCompRatio, 1 }, "Comp Ratio", juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f), 4.0f,
        Attributes().withStringFromValueFunction([](float ratio, int) { return juce::String(ratio, 1) + ":1"; })));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kCompAttack, 1 }, "Comp Attack", juce::NormalisableRange<float>(0.1f, 50.0f, 0.1f, 0.35f), 5.0f,
        milliseconds));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kCompRelease, 1 }, "Comp Release", juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f, 0.35f), 80.0f,
        milliseconds));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kCompMakeup, 1 }, "Comp Makeup", juce::NormalisableRange<float>(0.0f, 24.0f, 0.5f), 3.0f,
        decibels));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kCompMix, 1 }, "Comp Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f, percent));

    // Limiter
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kLimiterCeiling, 1 }, "Limiter Ceiling", juce::NormalisableRange<float>(-12.0f, 0.0f, 0.1f), -0.3f,
        decibels));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kLimiterRelease, 1 }, "Limiter Release", juce::NormalisableRange<float>(5.0f, 100.0f, 0.5f), 20.0f,
        milliseconds));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ kLimiterEnabled, 1 }, "Limiter Enabled", true));

    // Trance Gated Reverb
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kReverbSize, 1 }, "Reverb Size", juce::NormalisableRange<float>(0.1f, 1.0f, 0.01f), 0.65f, percent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kReverbDecay, 1 }, "Reverb Decay", juce::NormalisableRange<float>(50.0f, 2500.0f, 10.0f, 0.35f), 680.0f,
        milliseconds));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kReverbDamp, 1 }, "Reverb Damp", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.4f, percent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kReverbGate, 1 }, "Reverb Gate", juce::NormalisableRange<float>(0.0f, 1400.0f, 5.0f), 280.0f,
        milliseconds.withStringFromValueFunction([](float ms, int) {
            if (ms <= 0.0f) return juce::String("Off");
            return formatMilliseconds(ms);
        })));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kReverbMix, 1 }, "Reverb Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.35f, percent));

    juce::StringArray slapbackDivisions{ "Off", "1/16", "Dotted 1/16", "1/8", "Dotted 1/8", "1/4" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ kSlapbackDivision, 1 }, "Slapback Division", slapbackDivisions, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kSlapbackMix, 1 }, "Slapback Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f, percent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kSlapbackFeedback, 1 }, "Slapback Feedback", juce::NormalisableRange<float>(0.0f, 0.85f, 0.01f), 0.0f, percent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kClapDoubler, 1 }, "Stereo Doubler", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f, percent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kPitchHumanize, 1 }, "Pitch Humanize", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.0f,
        Attributes().withStringFromValueFunction([](float cents, int) { return juce::String(juce::roundToInt(cents)) + " ct"; })));

    // Reverse & Sync
    juce::StringArray revModes{ "Off", "Beat End (Sync)", "Beat 4 Auto", "1/2 Beat", "1/4 Beat" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ kReverseMode, 1 }, "Reverse Mode", revModes, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kReverseMix, 1 }, "Reverse Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f, percent));

    // Global / Master
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kStereoWidth, 1 }, "Master Width", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f, percent));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kOutputGain, 1 }, "Output Gain", juce::NormalisableRange<float>(-36.0f, 12.0f, 0.5f), 0.0f,
        decibels));
    const auto makeNoteRange = [](int minMidi, int maxMidi) {
        const float minHz = eqNoteToHz(minMidi);
        const float maxHz = eqNoteToHz(maxMidi);
        return juce::NormalisableRange<float>(
            minHz,
            maxHz,
            [minMidi, maxMidi](float, float, float proportion) {
                const float clamped = std::clamp(proportion, 0.0f, 1.0f);
                const int midi = juce::roundToInt(static_cast<float>(minMidi)
                    + clamped * static_cast<float>(maxMidi - minMidi));
                return eqNoteToHz(std::clamp(midi, minMidi, maxMidi));
            },
            [minMidi, maxMidi](float, float, float value) {
                const int midi = eqHzToNearestMidi(value, minMidi, maxMidi);
                return static_cast<float>(midi - minMidi) / static_cast<float>(maxMidi - minMidi);
            },
            [minMidi, maxMidi](float, float, float value) {
                return eqNoteToHz(eqHzToNearestMidi(value, minMidi, maxMidi));
            });
    };
    const auto makeNoteText = [](int minMidi, int maxMidi) {
        return Attributes()
            .withStringFromValueFunction([minMidi, maxMidi](float hz, int) { return formatEqNote(hz, minMidi, maxMidi); })
            .withValueFromStringFunction([minMidi, maxMidi](const juce::String& text) { return parseEqNoteOrHz(text, minMidi, maxMidi); });
    };
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kEqLowFreq, 1 }, "EQ Low Freq", makeNoteRange(kEqLowMinMidi, kEqLowMaxMidi), eqNoteToHz(40),
        makeNoteText(kEqLowMinMidi, kEqLowMaxMidi)));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kEqLowDb, 1 }, "EQ Low", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        decibels));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kEqMidFreq, 1 }, "EQ Mid Freq", makeNoteRange(kEqMidMinMidi, kEqMidMaxMidi), eqNoteToHz(83),
        makeNoteText(kEqMidMinMidi, kEqMidMaxMidi)));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kEqMidDb, 1 }, "EQ Mid", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        decibels));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kEqHighFreq, 1 }, "EQ High Freq", makeNoteRange(kEqHighMinMidi, kEqHighMaxMidi), eqNoteToHz(118),
        makeNoteText(kEqHighMinMidi, kEqHighMaxMidi)));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ kEqHighDb, 1 }, "EQ High", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        decibels));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{ kPreset, 1 }, "Preset Index", 0, kHostProgramCount - 1, 0));

    for (size_t layer = 0; layer < kLayerNames.size(); ++layer) {
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{kLayerMute[layer], 1}, juce::String(kLayerNames[layer]) + " Mute", false));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{kLayerSolo[layer], 1}, juce::String(kLayerNames[layer]) + " Solo", false));
        if (layer < kLayerVelocity.size())
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID{kLayerVelocity[layer], 1}, juce::String(kLayerNames[layer]) + " Velocity Response",
                juce::NormalisableRange<float>(0, 2, 0.01f), 1.0f, percent));
    }
    for (const auto& parameter : kPerformanceParameters) {
        if (juce::String(parameter.id) == "amp_enabled") {
            params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{parameter.id, 1}, parameter.name, false));
            continue;
        }
        const bool continuous = juce::String(parameter.id).startsWith("amp_");
        const bool sustain = juce::String(parameter.id) == "amp_sustain";
        const float step = sustain ? 0.01f : 1.0f;
        if (continuous) {
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID{parameter.id, 1}, parameter.name,
                juce::NormalisableRange<float>(parameter.minimum, parameter.maximum, step, parameter.maximum > 1 ? 0.35f : 1.0f), parameter.initial,
                sustain ? percent : milliseconds));
            continue;
        }
        params.push_back(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID{parameter.id, 1}, parameter.name, static_cast<int>(parameter.minimum),
            static_cast<int>(parameter.maximum), static_cast<int>(parameter.initial)));
    }
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"roll_division", 1}, "Roll Division",
        juce::StringArray{"Off", "1/8", "1/16", "1/32", "1/16 Triplet"}, 0));
    return { params.begin(), params.end() };
}

openclapper::core::EngineSettings readSettings(const juce::AudioProcessorValueTreeState& apvts) {
    openclapper::core::EngineSettings s;

    // Pre-Clap Flam
    s.synth.preClapCount = static_cast<int>(*apvts.getRawParameterValue(kPreBursts));
    s.synth.preClapSpreadMs = std::min(openclapper::core::kMaxTimingOffsetMs,
        std::max(0.0f, apvts.getRawParameterValue(kPreSpread)->load()));
    s.synth.preClapDecayMs = *apvts.getRawParameterValue(kPreDecay);
    s.synth.preClapJitter = *apvts.getRawParameterValue(kPreJitter);
    if (const auto* pFlamSpread = apvts.getRawParameterValue(kFlamSpread)) {
        s.synth.preClapFlamSpread = *pFlamSpread;
    }
    s.synth.preClapLevel = *apvts.getRawParameterValue(kPreLevel);

    // Resonant Body / Tone
    s.synth.toneQ = *apvts.getRawParameterValue(kToneQ);
    s.synth.toneDecayMs = *apvts.getRawParameterValue(kToneDecay);
    s.synth.tonePitchEnv = *apvts.getRawParameterValue(kTonePitchEnv);
    s.synth.tonePitchDecayMs = *apvts.getRawParameterValue(kTonePitchDecay);
    s.synth.toneHPF = *apvts.getRawParameterValue(kToneHPF);
    s.synth.toneLPF = *apvts.getRawParameterValue(kToneLPF);
    s.synth.toneLevel = *apvts.getRawParameterValue(kToneLevel);
    if (const auto* pImpact = apvts.getRawParameterValue(kImpactPunch)) {
        s.synth.impactPunch = *pImpact;
    }

    // Noise & Sizzle
    s.synth.noiseType = static_cast<openclapper::core::NoiseType>(static_cast<int>(*apvts.getRawParameterValue(kNoiseType)));
    s.synth.noiseAttackMs = std::min(openclapper::core::kMaxTimingOffsetMs,
        std::max(0.0f, apvts.getRawParameterValue(kNoiseAttack)->load()));
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
    s.synth.clickDecayMs = *apvts.getRawParameterValue(kClickDecay);

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
    if (const auto* pFeedback = apvts.getRawParameterValue(kSlapbackFeedback)) {
        s.effects.slapbackFeedback = *pFeedback;
    }
    if (const auto* pDoubler = apvts.getRawParameterValue(kClapDoubler)) {
        s.effects.doublerAmount = *pDoubler;
    }
    if (const auto* pPitchHum = apvts.getRawParameterValue(kPitchHumanize)) {
        s.synth.pitchHumanizeCents = *pPitchHum;
    }

    s.effects.reverbSize = *apvts.getRawParameterValue(kReverbSize);
    s.effects.reverbDecayMs = *apvts.getRawParameterValue(kReverbDecay);
    s.effects.reverbDamp = *apvts.getRawParameterValue(kReverbDamp);
    s.effects.reverbGateMs = *apvts.getRawParameterValue(kReverbGate);
    s.effects.reverbMix = *apvts.getRawParameterValue(kReverbMix);

    s.effects.reverseMode = static_cast<openclapper::core::ReverseSyncMode>(static_cast<int>(*apvts.getRawParameterValue(kReverseMode)));
    s.effects.reverseMix = *apvts.getRawParameterValue(kReverseMix);

    s.effects.eq.lowFreqHz = *apvts.getRawParameterValue(kEqLowFreq);
    s.effects.eq.lowGainDb = *apvts.getRawParameterValue(kEqLowDb);
    s.effects.eq.midFreqHz = *apvts.getRawParameterValue(kEqMidFreq);
    s.effects.eq.midGainDb = *apvts.getRawParameterValue(kEqMidDb);
    s.effects.eq.highFreqHz = *apvts.getRawParameterValue(kEqHighFreq);
    s.effects.eq.highGainDb = *apvts.getRawParameterValue(kEqHighDb);
    s.effects.stereoWidth = *apvts.getRawParameterValue(kStereoWidth);
    s.effects.outputGainDb = *apvts.getRawParameterValue(kOutputGain);

    for (size_t layer = 0; layer < kLayerNames.size(); ++layer) {
        s.synth.layers[layer].mute = *apvts.getRawParameterValue(kLayerMute[layer]) > 0.5f;
        s.synth.layers[layer].solo = *apvts.getRawParameterValue(kLayerSolo[layer]) > 0.5f;
        if (layer < kLayerVelocity.size())
            s.synth.layers[layer].velocityResponse = *apvts.getRawParameterValue(kLayerVelocity[layer]);
    }
    s.keyLow = static_cast<int>(*apvts.getRawParameterValue("key_low"));
    s.keyHigh = static_cast<int>(*apvts.getRawParameterValue("key_high"));
    s.velocityLow = static_cast<int>(*apvts.getRawParameterValue("velocity_low"));
    s.velocityHigh = static_cast<int>(*apvts.getRawParameterValue("velocity_high"));
    s.chokeGroup = static_cast<int>(*apvts.getRawParameterValue("choke_group"));
    s.rollDivision = static_cast<int>(*apvts.getRawParameterValue("roll_division"));
    s.rollHits = static_cast<int>(*apvts.getRawParameterValue("roll_hits"));
    s.synth.ampEnabled = *apvts.getRawParameterValue("amp_enabled") > 0.5f;
    s.synth.ampAttackMs = *apvts.getRawParameterValue("amp_attack");
    s.synth.ampDecayMs = *apvts.getRawParameterValue("amp_decay");
    s.synth.ampSustain = *apvts.getRawParameterValue("amp_sustain");
    s.synth.ampReleaseMs = *apvts.getRawParameterValue("amp_release");
    return s;
}

} // namespace openclapper::params
