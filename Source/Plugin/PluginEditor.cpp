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
        if (selected >= 0 && selected < static_cast<int>(openclapper::presets::factoryPresets().size())) {
            processor_.selectPreset(selected);
        }
    };
    syncPresetBox();
    addAndMakeVisible(presetBox_);

    prevPresetBtn_.onClick = [this]() {
        const int current = processor_.getSelectedPresetIndex();
        const int prev = (current - 1 + static_cast<int>(openclapper::presets::factoryPresets().size())) % static_cast<int>(openclapper::presets::factoryPresets().size());
        processor_.selectPreset(prev);
        syncPresetBox();
    };
    addAndMakeVisible(prevPresetBtn_);

    nextPresetBtn_.onClick = [this]() {
        const int current = processor_.getSelectedPresetIndex();
        const int next = (current + 1) % static_cast<int>(openclapper::presets::factoryPresets().size());
        processor_.selectPreset(next);
        syncPresetBox();
    };
    addAndMakeVisible(nextPresetBtn_);

    // Mode toggle button (Simple vs Advanced)
    viewToggleBtn_.setButtonText(synthPanel_.isAdvancedMode() ? "ADVANCED" : "SIMPLE");
    viewToggleBtn_.setColour(juce::TextButton::buttonColourId, openclapper::theme::kKnobBody);
    viewToggleBtn_.setColour(juce::TextButton::textColourOffId, openclapper::theme::kAccentCyan);
    viewToggleBtn_.setTooltip("Toggle between clean Simple mode and deep Advanced tweak controls.");
    viewToggleBtn_.onClick = [this]() {
        const bool nextAdv = !synthPanel_.isAdvancedMode();
        synthPanel_.setAdvancedMode(nextAdv);
        effectsPanel_.setAdvancedMode(nextAdv);
        viewToggleBtn_.setButtonText(nextAdv ? "ADVANCED" : "SIMPLE");
        viewToggleBtn_.setColour(juce::TextButton::textColourOffId,
            nextAdv ? openclapper::theme::kAccentAmber : openclapper::theme::kAccentCyan);
        resized();
    };
    addAndMakeVisible(viewToggleBtn_);

    addAndMakeVisible(stagePanel_);
    addAndMakeVisible(synthPanel_);
    addAndMakeVisible(effectsPanel_);

    const auto addControl = [this](std::unique_ptr<juce::Component> control) {
        performancePanel_.addAndMakeVisible(*control);
        performanceControls_.push_back(std::move(control));
    };
    using namespace openclapper::params;
    for (size_t layer = 0; layer < kLayerNames.size(); ++layer) {
        addControl(std::make_unique<openclapper::ui::Toggle>(processor_.parameters, kLayerMute[layer],
            juce::String(kLayerNames[layer]) + " Mute", openclapper::theme::kAccentAmber));
        addControl(std::make_unique<openclapper::ui::Toggle>(processor_.parameters, kLayerSolo[layer],
            juce::String(kLayerNames[layer]) + " Solo", openclapper::theme::kAccentCyan));
        if (layer < kLayerVelocity.size()) {
            auto knob = std::make_unique<openclapper::ui::Knob>(processor_.parameters, kLayerVelocity[layer],
                juce::String(kLayerNames[layer]) + " Velocity", openclapper::theme::kAccentCyan);
            knob->setTooltip("0 ignores velocity, 1 follows velocity, 2 gives a stronger velocity response.");
            addControl(std::move(knob));
        }
    }
    for (const auto& parameter : kPerformanceParameters) {
        if (juce::String(parameter.id) == "amp_enabled") {
            addControl(std::make_unique<openclapper::ui::Toggle>(processor_.parameters, parameter.id, parameter.name,
                openclapper::theme::kAccentAmber));
            continue;
        }
        addControl(std::make_unique<openclapper::ui::Knob>(processor_.parameters, parameter.id, parameter.name,
            openclapper::theme::kAccentCyan));
    }
    addControl(std::make_unique<openclapper::ui::Choice>(processor_.parameters, "roll_division", "Roll Division",
        openclapper::theme::kAccentCyan));
    addChildComponent(performancePanel_);
    performanceButton_.setColour(juce::TextButton::textColourOffId, openclapper::theme::kAccentCyan);
    performanceButton_.setColour(juce::TextButton::textColourOnId, openclapper::theme::kAccentAmber);
    performanceButton_.setClickingTogglesState(true);
    performanceButton_.onClick = [this] {
        const bool visible = performanceButton_.getToggleState();
        performancePanel_.setVisible(visible);
        stagePanel_.setVisible(!visible);
        synthPanel_.setVisible(!visible);
        effectsPanel_.setVisible(!visible);
        viewToggleBtn_.setEnabled(!visible);
        resized();
    };
    addAndMakeVisible(performanceButton_);
    setSize(1100, 740);
    setResizable(false, false);
    startTimerHz(20);
}

OpenClapperEditor::~OpenClapperEditor() {
    stopTimer();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void OpenClapperEditor::syncPresetBox() {
    presetBox_.setSelectedId(processor_.getSelectedPresetIndex() + 1, juce::dontSendNotification);
}

void OpenClapperEditor::timerCallback() {
    if (presetBox_.getSelectedId() != processor_.getSelectedPresetIndex() + 1) {
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
    g.drawText("v" JucePlugin_VersionString " | MIDI-TUNED TRANCE CLAP", getWidth() - 250, headerArea.getY() + 13, 234, 20, juce::Justification::right, true);
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
    viewToggleBtn_.setBounds(nextPresetBtn_.getRight() + 12, presetArea.getY() + 4, 86, 26);

    performanceButton_.setBounds(prevPresetBtn_.getX() - 106, presetArea.getY() + 4, 100, 26);
    performancePanel_.setBounds(area);
    const int cellWidth = area.getWidth() / 6;
    const int cellHeight = area.getHeight() / 4;
    for (size_t index = 0; index < performanceControls_.size(); ++index)
        performanceControls_[index]->setBounds(static_cast<int>(index % 6) * cellWidth + 8,
            static_cast<int>(index / 6) * cellHeight + 8, cellWidth - 16, cellHeight - 16);

    // Main Layout:
    // Left side: ClappyStagePanel (approx 340 px width)
    // Right side: Top = ClapSynthPanel, Bottom = EffectsPanel
    const int stageWidth = 330;
    stagePanel_.setBounds(area.removeFromLeft(stageWidth).reduced(2, 2));

    auto rightArea = area.reduced(2, 2);
    const float synthRatio = synthPanel_.isAdvancedMode() ? 0.58f : 0.50f;
    const int synthHeight = static_cast<int>(static_cast<float>(rightArea.getHeight()) * synthRatio);
    synthPanel_.setBounds(rightArea.removeFromTop(synthHeight).reduced(0, 2));
    effectsPanel_.setBounds(rightArea.reduced(0, 2));
}
