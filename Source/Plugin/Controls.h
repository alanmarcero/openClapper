#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace openclapper::ui {

using Apvts = juce::AudioProcessorValueTreeState;

/** Small clickable padlock icon for locking/unlocking an individual parameter against preset changes. */
class LockBadge final : public juce::Component, public juce::SettableTooltipClient, private juce::Timer {
public:
    LockBadge(Apvts& state, const juce::String& parameterId);
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit(const juce::MouseEvent&) override { repaint(); }

private:
    void timerCallback() override;
    Apvts& state_;
    juce::String id_;
    bool lastLocked_ = false;
};

class Control : public juce::Component {
public:
    Control(Apvts&, const char* id, const char* caption);
    void resized() override;
private:
    juce::Label label_;
    std::unique_ptr<juce::Slider> slider_;
    std::unique_ptr<juce::ComboBox> choice_;
    std::unique_ptr<juce::TextButton> toggle_;
    std::unique_ptr<Apvts::SliderAttachment> sliderAttachment_;
    std::unique_ptr<Apvts::ComboBoxAttachment> choiceAttachment_;
    std::unique_ptr<Apvts::ButtonAttachment> toggleAttachment_;
    std::unique_ptr<LockBadge> lock_;
};

class NoteGate : public juce::Component {
public:
    explicit NoteGate(Apvts&);
    void resized() override;
private:
    juce::TextButton oneShot_{ "One-shot" }, held_{ "Hold notes" };
    juce::ParameterAttachment attachment_;
    LockBadge lock_;
};

/** Dual concentric EQ knob: inner cap sets -12..+12 dB, outer collar glides Hz smoothly. */
class DualEqKnob final : public juce::Component {
public:
    DualEqKnob(Apvts& state, const juce::String& freqId, const juce::String& dbId,
               const juce::String& title);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;

private:
    class GainSlider final : public juce::Slider {
    public:
        using juce::Slider::Slider;
        bool hitTest(int x, int y) override;
    };

    [[nodiscard]] juce::Rectangle<float> dialBounds() const;
    [[nodiscard]] juce::Rectangle<int> dbPillBounds() const;
    [[nodiscard]] juce::Rectangle<int> freqPillBounds() const;
    [[nodiscard]] bool hitsOuterRing(juce::Point<float> pos) const;
    void updateDbReadout();

    Apvts& state_;
    juce::String freqId_;
    juce::String dbId_;
    juce::Label title_;
    juce::Label dbReadout_;
    GainSlider dbSlider_;
    std::unique_ptr<Apvts::SliderAttachment> dbAttachment_;
    std::unique_ptr<juce::ParameterAttachment> freqAttachment_;
    LockBadge dbLock_;
    LockBadge freqLock_;
    float freqHz_ = 1000.0f;
    float dragStartNorm_ = 0.5f;
    float dragStartY_ = 0.0f;
    bool draggingFreq_ = false;
    bool hoveringFreq_ = false;
};

} // namespace openclapper::ui
