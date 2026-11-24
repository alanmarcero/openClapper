#pragma once

#include "EdmLookAndFeel.h"
#include "Panels.h"
#include "Parameters.h"
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
    void refreshChrome();
    void stepPreset(int delta);
    void setPage(int index);
    void showPresetBrowser();
    void setBrowserVisible(bool visible);

    OpenClapperProcessor& processor_;
    openclapper::ui::EdmLookAndFeel look_;
    // Holds every control at the design size. resized() scales it to fit the window.
    juce::Component content_;
    juce::ComboBox presets_;
    openclapper::ui::LockBadge presetLock_{ processor_.parameters, openclapper::params::kPreset, "Preset" };
    struct PresetStepButton final : juce::Button {
        PresetStepButton(const juce::String& name, float direction)
            : juce::Button(name), direction_(direction) {}
        void paintButton(juce::Graphics& g, bool highlighted, bool down) override {
            getLookAndFeel().drawButtonBackground(g, *this, findColour(juce::TextButton::buttonColourId), highlighted, down);
            const auto c = getLocalBounds().toFloat().getCentre();
            juce::Path arrow;
            arrow.startNewSubPath(c.x + direction_ * 3.0f, c.y - 5.0f);
            arrow.lineTo(c.x - direction_ * 3.0f, c.y);
            arrow.lineTo(c.x + direction_ * 3.0f, c.y + 5.0f);
            g.setColour(findColour(juce::TextButton::textColourOffId));
            g.strokePath(arrow, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        float direction_;
    };

    PresetStepButton previous_{ "Previous", 1.0f }, next_{ "Next", -1.0f };
    juce::TextButton audition_{ "Audition A4" };
    juce::TextButton lock_{ "Lock in" };
    openclapper::ui::LockBadge lockBadge_{ processor_.parameters, openclapper::params::kLockIn, "Lock in" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lockAttachment_;
    suite::SuiteBar suite_;
    suite::PresetBrowser browser_;
    std::array<juce::TextButton, 4> tabs_;
    std::array<std::unique_ptr<openclapper::ui::ControlPage>, 4> pages_;
    openclapper::ui::Waveform waveform_;
    openclapper::ui::OutputMeter meter_;
    openclapper::ui::HitLight hitLight_;
    std::unique_ptr<juce::Drawable> hand_;
    juce::TooltipWindow tooltips_{this, 500};
    int page_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenClapperEditor)
};
