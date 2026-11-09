#pragma once

#include "Controls.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class OpenClapperProcessor;

namespace openclapper::ui {

class ClappyStagePanel : public juce::Component, private juce::Timer {
public:
    explicit ClappyStagePanel(OpenClapperProcessor& processor);
    ~ClappyStagePanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    OpenClapperProcessor& processor_;
    juce::TextButton auditionBtn_{ "CLAP ON!" };
    uint32_t seenTriggers_ = 0;
    double clapTriggerTimeMs_ = 0.0;
    float clapPose_ = 0.0f; // -1 (open flam) .. 0 (slap) .. +1 (recoil)
    float shockwave_ = 0.0f;
    float impactIntensity_ = 1.0f;
    float impactFlash_ = 0.0f;
    float headTilt_ = 0.0f;
    float idleBob_ = 0.0f;
};

class ClapSynthPanel : public juce::Component {
public:
    explicit ClapSynthPanel(OpenClapperProcessor& processor);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void setAdvancedMode(bool advanced);
    [[nodiscard]] bool isAdvancedMode() const noexcept { return advancedMode_; }

private:
    bool advancedMode_ = false;

    // Pre-Clap Flam
    Knob preBursts_;
    Knob preSpread_;
    Knob flamSpread_;
    Knob preLevel_;

    // Resonant Body / Tone
    Choice toneMode_;
    Choice toneNote_;
    Knob toneOctave_;
    Knob toneFine_;
    Toggle lockPitchBtn_;
    Knob toneQ_;
    Knob toneDecay_;
    Knob tonePitchEnv_;
    Knob toneLevel_;

    // Noise & Sizzle
    Choice noiseType_;
    Knob noiseDecay_;
    Knob noiseHPF_;
    Knob noiseLPF_;
    Knob noiseWidth_;
    Knob noiseLevel_;

    // Click
    Choice clickType_;
    Knob clickLevel_;
    Knob clickFreq_;
};

class EffectsPanel : public juce::Component {
public:
    explicit EffectsPanel(OpenClapperProcessor& processor);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void setAdvancedMode(bool advanced);
    [[nodiscard]] bool isAdvancedMode() const noexcept { return advancedMode_; }

private:
    bool advancedMode_ = false;

    // Distortion
    Choice distType_;
    Knob distDrive_;
    Knob distTone_;
    Knob distMix_;

    // Compressor
    Knob compThresh_;
    Knob compRatio_;
    Knob compMakeup_;
    Knob compMix_;

    // Limiter
    Knob limiterCeiling_;
    Toggle limiterEnabled_;

    // Gated Reverb
    Knob reverbSize_;
    Knob reverbDecay_;
    Knob reverbGate_;
    Knob reverbMix_;

    // Reverse & Master
    Choice reverseMode_;
    Knob reverseMix_;
    Choice midiMode_;
    Toggle lockInBtn_;
    Knob masterWidth_;
    Knob outputGain_;
};

} // namespace openclapper::ui
