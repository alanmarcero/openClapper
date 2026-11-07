#include "PluginEditor.h"
#include "BinaryData.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"

OpenClapperEditor::OpenClapperEditor(OpenClapperProcessor& processorToEdit)
    : AudioProcessorEditor(&processorToEdit),
      processor_(processorToEdit),
      stagePanel_(processorToEdit),
      synthPanel_(processorToEdit),
      effectsPanel_(processorToEdit),
      tooltipWindow_(this, 500) {
    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel_);

    faceplate_ = juce::ImageCache::getFromMemory(
        BinaryData::openclapperfaceplate_png, BinaryData::openclapperfaceplate_pngSize);

    // Preset selector
    const auto& presets = openclapper::presets::factoryPresets();
    for (size_t i = 0; i < presets.size(); ++i) {
        presetBox_.addItem(presets[i].name, static_cast<int>(i + 1));
    }
    presetBox_.onChange = [this]() {
        const int selected = presetBox_.getSelectedId() - 1;
        if (selected >= 0 && selected < processor_.getNumPrograms()) {
            processor_.setCurrentProgram(selected);
        }
    };
    syncPresetBox();
    addAndMakeVisible(presetBox_);

    prevPresetBtn_.onClick = [this]() {
        const int current = processor_.getCurrentProgram();
        const int prev = (current - 1 + processor_.getNumPrograms()) % processor_.getNumPrograms();
        processor_.setCurrentProgram(prev);
        syncPresetBox();
    };
    addAndMakeVisible(prevPresetBtn_);

    nextPresetBtn_.onClick = [this]() {
        const int current = processor_.getCurrentProgram();
        const int next = (current + 1) % processor_.getNumPrograms();
        processor_.setCurrentProgram(next);
        syncPresetBox();
    };
    addAndMakeVisible(nextPresetBtn_);

    addAndMakeVisible(stagePanel_);
    addAndMakeVisible(synthPanel_);
    addAndMakeVisible(effectsPanel_);

    setSize(1100, 740);
    setResizable(false, false);
    startTimerHz(20);
}

OpenClapperEditor::~OpenClapperEditor() {
    stopTimer();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void OpenClapperEditor::syncPresetBox() {
    presetBox_.setSelectedId(processor_.getCurrentProgram() + 1, juce::dontSendNotification);
}

void OpenClapperEditor::timerCallback() {
    if (presetBox_.getSelectedId() != processor_.getCurrentProgram() + 1) {
        syncPresetBox();
    }
}

void OpenClapperEditor::paint(juce::Graphics& g) {
    if (faceplate_.isValid()) {
        g.drawImageWithin(faceplate_, 0, 0, getWidth(), getHeight(),
                          juce::RectanglePlacement::stretchToFit);
    } else {
        g.fillAll(openclapper::theme::kBackground);
    }

    // Top Header Banner
    const auto headerArea = getLocalBounds().removeFromTop(44).reduced(12, 4);

    // Logo & Title
    g.setColour(openclapper::theme::kAccentCyan);
    g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    g.drawText("openClapper", headerArea.getX() + 4, headerArea.getY() + 2, 160, 24, juce::Justification::left, true);

    // Slogan in Header
    g.setColour(openclapper::theme::kAccentAmber);
    g.setFont(juce::FontOptions(13.0f, juce::Font::italic));
    g.drawText("Clap on. Clap off.", headerArea.getX() + 168, headerArea.getY() + 6, 160, 20, juce::Justification::left, true);

    // Tech badges
    g.setColour(openclapper::theme::kTextDim);
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("v1.1.1 | EDM & TRANCE CLAP SYNTH", headerArea.getRight() - 250, headerArea.getY() + 6, 240, 20, juce::Justification::right, true);
}

void OpenClapperEditor::resized() {
    auto area = getLocalBounds().reduced(10, 8);

    // Top Header
    auto header = area.removeFromTop(38);
    const int presetW = 220;
    auto presetArea = header.withX(header.getCentreX() - presetW / 2).withWidth(presetW);
    prevPresetBtn_.setBounds(presetArea.getX() - 28, presetArea.getY() + 4, 24, 26);
    presetBox_.setBounds(presetArea.withHeight(26).translated(0, 4));
    nextPresetBtn_.setBounds(presetArea.getRight() + 4, presetArea.getY() + 4, 24, 26);

    // Main Layout:
    // Left side: ClappyStagePanel (approx 340 px width)
    // Right side: Top = ClapSynthPanel (3 rows of knobs), Bottom = EffectsPanel (2 rows of knobs)
    const int stageWidth = 330;
    stagePanel_.setBounds(area.removeFromLeft(stageWidth).reduced(2, 2));

    auto rightArea = area.reduced(2, 2);
    const int synthHeight = static_cast<int>(rightArea.getHeight() * 0.58f);
    synthPanel_.setBounds(rightArea.removeFromTop(synthHeight).reduced(0, 2));
    effectsPanel_.setBounds(rightArea.reduced(0, 2));
}
