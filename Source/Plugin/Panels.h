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
    juce::TextButton auditionBtn_{ "PUSH IT" };
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
    Knob toneQ_;
    Knob toneDecay_;
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

    juce::Rectangle<int> attackSectionBounds_;
    juce::Rectangle<int> tailSectionBounds_;
    juce::Rectangle<int> noiseSectionBounds_;
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
    Choice slapbackDivision_;
    Knob slapbackMix_;
    Knob masterWidth_;
    Knob outputGain_;
};

} // namespace openclapper::ui
