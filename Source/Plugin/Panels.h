#pragma once

#include "ClapSynth.h"
#include "Controls.h"
#include <optional>

class OpenClapperProcessor;

namespace openclapper::ui {

struct ControlSpec { const char* id; const char* caption; };

// Why a control has no audible effect in the current state, or an empty string when it has one.
juce::String inactiveReason(const Apvts& state, const juce::String& id);

class ControlGroup : public juce::Component, private juce::Timer {
public:
    ControlGroup(Apvts&, const char* heading, std::initializer_list<ControlSpec>, int columns, bool stacked,
                 bool outputSection = false);
    void paint(juce::Graphics&) override;
    void resized() override;
    int rows() const;
private:
    void timerCallback() override;
    void refreshLockButton();
    Apvts& state_;
    juce::String heading_;
    int columns_;
    bool stacked_;
    bool outputSection_ = false;
    juce::StringArray paramIds_;
    juce::TextButton lockButton_;
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
