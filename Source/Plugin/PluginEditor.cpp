#include "PluginEditor.h"
#include "Presets.h"
#include "BinaryData.h"
#include "Theme.h"

using namespace openclapper;

OpenClapperEditor::OpenClapperEditor(OpenClapperProcessor& instrument)
    : AudioProcessorEditor(&instrument), processor_(instrument), waveform_(instrument) {
    hand_ = juce::Drawable::createFromImageData(BinaryData::claphand_svg, BinaryData::claphand_svgSize);
    const auto& bank = presets::factoryPresets();
    for (size_t i = 0; i < bank.size(); ++i) presets_.addItem(bank[i].name, static_cast<int>(i + 1));
    presets_.setName("Factory preset");
    presets_.setTooltip("Choose a factory clap by family and name.");
    presets_.setTextWhenNothingSelected("Custom sound");
    presets_.onChange = [this] {
        if (presets_.getSelectedId() > 0) {
            processor_.selectPreset(presets_.getSelectedId() - 1);
            waveform_.refresh();
        }
    };
    addAndMakeVisible(presets_);
    previous_.setTooltip("Load the previous factory clap.");
    next_.setTooltip("Load the next factory clap.");
    audition_.setTooltip("Play the current sound at A4.");
    previous_.onClick = [this] { stepPreset(-1); };
    next_.onClick = [this] { stepPreset(1); };
    audition_.onClick = [this] { processor_.triggerAudition(); };
    audition_.setColour(juce::TextButton::buttonColourId, theme::apricot);
    audition_.setColour(juce::TextButton::textColourOffId, theme::plot);
    for (auto* button : { &previous_, &next_, &audition_ }) addAndMakeVisible(*button);

    constexpr std::array names{ "Clap", "Layers", "Effects", "MIDI & envelope" };
    for (size_t i = 0; i < tabs_.size(); ++i) {
        pages_[i] = std::make_unique<ui::ControlPage>(processor_.parameters, static_cast<ui::Page>(i));
        addChildComponent(*pages_[i]);
        tabs_[i].setButtonText(names[i]);
        tabs_[i].setTooltip("Open the " + juce::String(names[i]) + " page.");
        tabs_[i].onClick = [this, i] { setPage(static_cast<int>(i)); };
        addAndMakeVisible(tabs_[i]);
    }
    addAndMakeVisible(waveform_);
    setLookAndFeel(&look_);
    setSize(1080, 710);
    syncPreset();
    setPage(0);
    startTimerHz(24);
}

OpenClapperEditor::~OpenClapperEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void OpenClapperEditor::stepPreset(int delta) {
    const int size = presets_.getNumItems();
    const int current = processor_.getSelectedPresetIndex();
    const int next = current < 0 ? (delta > 0 ? 0 : size - 1) : (current + delta + size) % size;
    processor_.selectPreset(next);
    syncPreset();
    waveform_.refresh();
}

void OpenClapperEditor::syncPreset() {
    presets_.setSelectedId(processor_.getSelectedPresetIndex() + 1, juce::dontSendNotification);
}

void OpenClapperEditor::setPage(int index) {
    page_ = index;
    for (size_t i = 0; i < pages_.size(); ++i) {
        const bool active = static_cast<int>(i) == index;
        pages_[i]->setVisible(active);
        tabs_[i].setToggleState(active, juce::dontSendNotification);
    }
    waveform_.setVisible(page_ == 0);
    if (page_ == 0) waveform_.refresh();
    resized();
}

void OpenClapperEditor::timerCallback() {
    syncPreset();
    if (page_ == 0 && isShowing()) waveform_.refresh();
    repaint(getWidth() - 170, 89, 150, 59);
}

void OpenClapperEditor::paint(juce::Graphics& g) {
    g.fillAll(theme::paper);
    g.setColour(theme::surface);
    g.fillRect(0, 0, getWidth(), 88);
    g.setColour(theme::edge);
    g.drawHorizontalLine(87, 0, static_cast<float>(getWidth()));
    g.setColour(theme::violet);
    constexpr std::array<int, 3> lengths{24, 39, 30};
    for (size_t i = 0; i < lengths.size(); ++i) {
        const float x = 21.0f + static_cast<float>(i) * 9.0f;
        const float h = static_cast<float>(lengths[i]);
        juce::Path stroke;
        stroke.startNewSubPath(x, 44.0f + h * 0.5f);
        stroke.lineTo(x + 9, 44.0f - h * 0.5f);
        g.strokePath(stroke, juce::PathStrokeType(5.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    g.setColour(theme::ink);
    g.setFont(juce::FontOptions(32.0f, juce::Font::bold));
    g.drawText("openClapper", 62, 11, 200, 40, juce::Justification::centredLeft);
    g.setColour(theme::secondary);
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawText("v" JucePlugin_VersionString, 256, 20, 48, 24, juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(14.0f));
    g.drawText("Pitched clap synthesizer", 64, 47, 220, 22, juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawText("Factory preset", presets_.getX(), 8, 280, 20, juce::Justification::centredLeft);

    const float beat = processor_.displayBeatPhase.load(std::memory_order_relaxed);
    const bool running = beat >= 0.0f;
    const float contact = running ? std::exp(-7.0f * std::min(beat, 1.0f - beat)) : 0.0f;
    const float centre = static_cast<float>(getWidth() - 56);
    if (hand_) {
        for (float side : {-1.0f, 1.0f}) {
            const auto transform = juce::AffineTransform::translation(-20, -22)
                .scaled(-side * 0.72f, 0.72f)
                .rotated(side * (0.18f + 0.26f * contact))
                .translated(centre + side * (19.0f - 10.0f * contact), 118.0f);
            hand_->draw(g, running ? 1.0f : 0.6f, transform);
        }
    }
    g.setColour(theme::secondary);
    g.drawText(running ? juce::String(juce::roundToInt(processor_.displayBpm.load())) + " BPM" : "Stopped",
               getWidth() - 164, 99, 64, 38, juce::Justification::centredRight);
}

void OpenClapperEditor::resized() {
    auto area = getLocalBounds().reduced(20, 0);
    auto header = area.removeFromTop(88).withTrimmedTop(30).withHeight(38);
    header.removeFromLeft(280);
    audition_.setBounds(header.removeFromRight(140));
    header.removeFromRight(14);
    next_.setBounds(header.removeFromRight(70));
    header.removeFromRight(5);
    previous_.setBounds(header.removeFromRight(91));
    header.removeFromRight(6);
    presets_.setBounds(header);

    auto nav = area.removeFromTop(61).reduced(0, 12);
    constexpr std::array widths{82, 95, 95, 160};
    for (size_t i = 0; i < tabs_.size(); ++i) {
        tabs_[i].setBounds(nav.removeFromLeft(widths[i]));
        nav.removeFromLeft(6);
    }
    area.removeFromBottom(16);
    if (page_ == 0) {
        waveform_.setBounds(area.removeFromTop(213));
        area.removeFromTop(10);
    }
    pages_[static_cast<size_t>(page_)]->setBounds(area);
}
