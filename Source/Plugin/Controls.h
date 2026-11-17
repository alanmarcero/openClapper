#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace openclapper::ui {

using Apvts = juce::AudioProcessorValueTreeState;

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
};

class NoteGate : public juce::Component {
public:
    explicit NoteGate(Apvts&);
    void resized() override;
private:
    juce::TextButton oneShot_{ "One-shot" }, held_{ "Hold notes" };
    juce::ParameterAttachment attachment_;
};

} // namespace openclapper::ui
