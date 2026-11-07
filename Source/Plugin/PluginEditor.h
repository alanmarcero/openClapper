#pragma once

#include "EdmLookAndFeel.h"
#include "Panels.h"
#include "PluginProcessor.h"

class OpenClapperEditor : public juce::AudioProcessorEditor,
                          private juce::Timer {
public:
    explicit OpenClapperEditor(OpenClapperProcessor& processorToEdit);
    ~OpenClapperEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void syncPresetBox();

    OpenClapperProcessor& processor_;
    openclapper::ui::EdmLookAndFeel lookAndFeel_;
    juce::Image faceplate_;

    juce::ComboBox presetBox_;
    juce::TextButton prevPresetBtn_{ "<" };
    juce::TextButton nextPresetBtn_{ ">" };

    openclapper::ui::ClappyStagePanel stagePanel_;
    openclapper::ui::ClapSynthPanel synthPanel_;
    openclapper::ui::EffectsPanel effectsPanel_;
    juce::TooltipWindow tooltipWindow_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenClapperEditor)
};
