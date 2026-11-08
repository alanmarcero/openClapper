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
    // 1. Solid dark chassis base
    g.fillAll(openclapper::theme::kBackground);

    // 2. Chassis texture in margins (high-tech carbon & cyber trace styling)
    if (faceplate_.isValid()) {
        const juce::Graphics::ScopedSaveState state(g);
        g.setOpacity(0.40f);
        g.drawImageWithin(faceplate_, 0, 0, getWidth(), getHeight(),
                          juce::RectanglePlacement::stretchToFit);
    }

    // 3. Solid top header bar with border and cyan accent strip
    const auto headerArea = getLocalBounds().removeFromTop(44);
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xff111822), 0.0f, 0.0f,
        juce::Colour(0xff090e15), 0.0f, static_cast<float>(headerArea.getHeight()), false));
    g.fillRect(headerArea);
    g.setColour(openclapper::theme::kPanelEdge);
    g.drawHorizontalLine(headerArea.getBottom() - 1, 0.0f, static_cast<float>(getWidth()));
    g.setColour(openclapper::theme::kAccentCyan.withAlpha(0.6f));
    g.drawHorizontalLine(headerArea.getBottom() - 1, 12.0f, 260.0f);

    // Logo & Title
    g.setColour(openclapper::theme::kAccentCyan);
    g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    g.drawText("openClapper", 16, headerArea.getY() + 10, 140, 24, juce::Justification::left, true);

    // Slogan in Header
    g.setColour(openclapper::theme::kAccentAmber);
    g.setFont(juce::FontOptions(13.0f, juce::Font::italic));
    g.drawText("Clap on. Clap off.", 160, headerArea.getY() + 13, 140, 20, juce::Justification::left, true);

    // Tech badges
    g.setColour(openclapper::theme::kTextDim.brighter(0.25f));
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText("v1.3.0 | EDM & TRANCE CLAP SYNTH", getWidth() - 250, headerArea.getY() + 13, 234, 20, juce::Justification::right, true);
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
    const int synthHeight = static_cast<int>(static_cast<float>(rightArea.getHeight()) * 0.58f);
    synthPanel_.setBounds(rightArea.removeFromTop(synthHeight).reduced(0, 2));
    effectsPanel_.setBounds(rightArea.reduced(0, 2));
}
