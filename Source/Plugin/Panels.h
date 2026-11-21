#pragma once

#include "ClapSynth.h"
#include "Controls.h"
#include <optional>

class OpenClapperProcessor;

namespace openclapper::ui {

struct ControlSpec { const char* id; const char* caption; };

enum class LayerState { audible, muted, notSoloed };

// Whether a layer sounds, from the layer Mute and Solo buttons.
LayerState layerState(const Apvts& state, size_t layer);

// Why a control has no audible effect in the current state, or an empty string when it has one.
juce::String inactiveReason(const Apvts& state, const juce::String& id);

class ControlGroup : public juce::Component, private juce::Timer {
public:
    // A card with a layer index gets Mute and Solo chips for that layer.
    ControlGroup(Apvts&, const char* heading, std::initializer_list<ControlSpec>, int columns, bool stacked,
                 bool outputSection = false, int layer = -1);
    void paint(juce::Graphics&) override;
    void resized() override;
    int rows() const;
    // Updates the Lock button and dims the controls that have no effect now.
    void refresh();
private:
    void timerCallback() override;
    void refreshLockButton();
    Apvts& state_;
    juce::String heading_;
    int columns_;
    bool stacked_;
    bool outputSection_ = false;
    int layer_ = -1;
    juce::String layerStatus_;
    // The Output card frames the limiter and shows where limited peaks land after Output gain.
    juce::Rectangle<int> limiterArea_;
    juce::Label outputCeiling_;
    juce::StringArray paramIds_;
    juce::TextButton lockButton_;
    std::array<juce::TextButton, 2> chips_;
    std::array<std::unique_ptr<Apvts::ButtonAttachment>, 2> chipAttachments_;
    std::vector<std::unique_ptr<juce::Component>> controls_;
};

enum class Page { clap, layers, effects, midi };

class ControlPage : public juce::Component {
public:
    ControlPage(Apvts&, Page);
    void resized() override;
private:
    Page page_;
    std::vector<std::unique_ptr<ControlGroup>> groups_;
};

class Waveform : public juce::Component {
public:
    explicit Waveform(OpenClapperProcessor&);
    void refresh();
    void paint(juce::Graphics&) override;
private:
    OpenClapperProcessor& processor_;
    std::optional<core::ClapSynthSettings> previous_;
    std::array<float, 560> upper_{}, lower_{};
    int durationMs_ = 200;
    float strikeMs_ = 0;
};

} // namespace openclapper::ui
