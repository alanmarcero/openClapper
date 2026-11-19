#pragma once

#include "EdmLookAndFeel.h"
#include "Panels.h"
#include "PluginProcessor.h"
#include "suite/PresetBrowser.h"
#include "suite/SuiteBar.h"

class OpenClapperEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit OpenClapperEditor(OpenClapperProcessor&);
    ~OpenClapperEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

private:
    void timerCallback() override;
    void syncPreset();
    void refreshChrome();
    void stepPreset(int delta);
    void setPage(int index);
    void showPresetBrowser();

    OpenClapperProcessor& processor_;
    openclapper::ui::EdmLookAndFeel look_;
    juce::ComboBox presets_;
    juce::TextButton previous_{ "Previous" }, next_{ "Next" }, audition_{ "Audition A4" };
    suite::SuiteBar suite_;
    suite::PresetBrowser browser_;
    std::array<juce::TextButton, 4> tabs_;
    std::array<std::unique_ptr<openclapper::ui::ControlPage>, 4> pages_;
    openclapper::ui::Waveform waveform_;
    std::unique_ptr<juce::Drawable> hand_;
    juce::TooltipWindow tooltips_{this, 500};
    int page_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenClapperEditor)
};
