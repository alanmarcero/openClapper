#include "PluginEditor.h"
#include "Presets.h"
#include "BinaryData.h"
#include "Theme.h"

using namespace openclapper;

namespace {
constexpr int kHeaderHeight = 88;
constexpr int kSuiteHeight = 44;
constexpr int kNavHeight = 61;
constexpr int kDefaultWidth = 1080;
constexpr int kDefaultHeight = 868;
constexpr int kMinWidth = 1080;
constexpr int kMinHeight = 868;
constexpr int kMaxWidth = 2400;
constexpr int kMaxHeight = 1800;

std::vector<juce::String> factoryNames() {
    std::vector<juce::String> names;
    for (const auto& preset : presets::factoryPresets()) names.emplace_back(preset.name);
    return names;
}
}

OpenClapperEditor::OpenClapperEditor(OpenClapperProcessor& instrument)
    : AudioProcessorEditor(&instrument), processor_(instrument),
      browser_(processor_.userPresets(), factoryNames()), waveform_(instrument) {
    hand_ = juce::Drawable::createFromImageData(BinaryData::claphand_svg, BinaryData::claphand_svgSize);
    const auto& bank = presets::factoryPresets();
    juce::String family;
    juce::PopupMenu sounds;
    int id = 1;
    std::ranges::for_each(bank, [&](const auto& preset) {
        const juce::String name(preset.name);
        const auto nextFamily = name.upToFirstOccurrenceOf(" / ", false, false);
        if (family != nextFamily && sounds.getNumItems() > 0) {
            presets_.getRootMenu()->addSubMenu(family, sounds);
            sounds.clear();
        }
        family = nextFamily;
        sounds.addItem(id++, name);
    });
    if (sounds.getNumItems() > 0) presets_.getRootMenu()->addSubMenu(family, sounds);
    presets_.setName("Factory preset");
    presets_.setTooltip("Choose a factory clap across six dance styles.");
    presets_.setTextWhenNothingSelected("Custom sound");
    presets_.onChange = [this] {
        if (presets_.getSelectedId() > 0) {
            processor_.selectPreset(presets_.getSelectedId() - 1);
            waveform_.refresh();
            browser_.setVisible(false);
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

    suite_.onUndo = [this] { processor_.undo(); refreshChrome(); };
    suite_.onRedo = [this] { processor_.redo(); refreshChrome(); };
    suite_.onShowA = [this] { if (processor_.isAbSlotB()) { processor_.toggleAb(); refreshChrome(); } };
    suite_.onShowB = [this] { if (!processor_.isAbSlotB()) { processor_.toggleAb(); refreshChrome(); } };
    suite_.onCopyAToB = [this] { processor_.copyAbAToB(); refreshChrome(); };
    suite_.onCopyBToA = [this] { processor_.copyAbBToA(); refreshChrome(); };
    suite_.onPresets = [this] { showPresetBrowser(); };
    suite_.onDragWav = [this] {
        const auto name = processor_.userPresetName().isNotEmpty()
            ? processor_.userPresetName()
            : (processor_.getSelectedPresetIndex() >= 0
                ? processor_.getProgramName(processor_.getSelectedPresetIndex())
                : juce::String("Custom sound"));
        auto stem = suite::UserPresetStore::sanitise(name);
        stem = stem.replaceCharacters("\\/:*?\"<>|", "_________");
        if (stem.isEmpty()) stem = "openClapper";
        auto file = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("Alan Marcero").getChildFile("openClapper").getChildFile(stem + ".wav");
        return processor_.renderHitWav(file);
    };
    addAndMakeVisible(suite_);

    browser_.setPalette({
        theme::paper, theme::surface, theme::ink, theme::secondary, theme::edge, theme::violet, theme::selected
    });
    browser_.onFactoryPreset = [this](int index) {
        processor_.selectPreset(index);
        refreshChrome();
    };
    browser_.onUserPreset = [this](const juce::String& name) {
        processor_.loadUserPreset(name);
        refreshChrome();
    };
    browser_.onSave = [this](const juce::String& name) { return processor_.saveUserPreset(name); };
    browser_.onRename = [this](const juce::String& from, const juce::String& to) {
        return processor_.renameUserPreset(from, to);
    };
    browser_.onDelete = [this](const juce::String& name) { return processor_.deleteUserPreset(name); };
    browser_.setVisible(false);
    addChildComponent(browser_);

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
    setWantsKeyboardFocus(true);
    const int savedWidth = processor_.editorWidth();
    const int savedHeight = processor_.editorHeight();
    setResizable(true, true);
    setResizeLimits(kMinWidth, kMinHeight, kMaxWidth, kMaxHeight);
    const int width = juce::jlimit(kMinWidth, kMaxWidth, savedWidth > 0 ? savedWidth : kDefaultWidth);
    const int height = juce::jlimit(kMinHeight, kMaxHeight, savedHeight > 0 ? savedHeight : kDefaultHeight);
    setSize(width, height);
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
    if (processor_.userPresetName().isNotEmpty()) {
        presets_.setSelectedId(0, juce::dontSendNotification);
        presets_.setText(processor_.userPresetName(), juce::dontSendNotification);
        return;
    }
    presets_.setSelectedId(processor_.getSelectedPresetIndex() + 1, juce::dontSendNotification);
}

void OpenClapperEditor::refreshChrome() {
    syncPreset();
    suite_.setAbSlotB(processor_.isAbSlotB());
    suite_.setUndoEnabled(processor_.canUndo(), processor_.canRedo());
    waveform_.refresh();
    repaint();
}

void OpenClapperEditor::showPresetBrowser() {
    browser_.reload();
    browser_.setVisible(!browser_.isVisible());
    if (browser_.isVisible()) browser_.toFront(false);
    resized();
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

bool OpenClapperEditor::keyPressed(const juce::KeyPress& key) {
    if (key == juce::KeyPress::escapeKey && browser_.isVisible()) {
        browser_.setVisible(false);
        return true;
    }
    const auto mods = key.getModifiers();
    const auto code = key.getKeyCode();
    if (mods.isCommandDown() && (code == 'Z' || code == 'z')) {
        if (mods.isShiftDown()) processor_.redo();
        else processor_.undo();
        refreshChrome();
        return true;
    }
    return false;
}

void OpenClapperEditor::timerCallback() {
    syncPreset();
    suite_.setAbSlotB(processor_.isAbSlotB());
    suite_.setUndoEnabled(processor_.canUndo(), processor_.canRedo());
    if (page_ == 0 && isShowing()) waveform_.refresh();
    const int handTop = kHeaderHeight + kSuiteHeight;
    repaint(getWidth() - 180, handTop, 170, 80);
}

void OpenClapperEditor::paint(juce::Graphics& g) {
    g.fillAll(theme::paper);
    const int chrome = kHeaderHeight + kSuiteHeight;
    g.setColour(theme::surface);
    g.fillRect(0, 0, getWidth(), chrome);
    g.setColour(theme::edge);
    g.drawHorizontalLine(chrome - 1, 0, static_cast<float>(getWidth()));
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
    g.drawText("v" JucePlugin_VersionString, 256, 20, 56, 24, juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(14.0f));
    g.drawText("Pitched clap synthesizer", 64, 47, 220, 22, juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawText("Preset", presets_.getX(), 8, 280, 20, juce::Justification::centredLeft);

    const float beat = processor_.displayBeatPhase.load(std::memory_order_relaxed);
    const bool running = beat >= 0.0f;
    const float contact = running ? std::exp(-7.0f * std::min(beat, 1.0f - beat)) : 0.0f;
    const float centre = static_cast<float>(getWidth() - 56);
    const float handY = static_cast<float>(kHeaderHeight + kSuiteHeight + 30);
    if (hand_) {
        for (float side : {-1.0f, 1.0f}) {
            const auto transform = juce::AffineTransform::translation(-20, -22)
                .scaled(-side * 0.72f, 0.72f)
                .rotated(side * (0.18f + 0.26f * contact))
                .translated(centre + side * (19.0f - 10.0f * contact), handY);
            hand_->draw(g, running ? 1.0f : 0.6f, transform);
        }
    }
    g.setColour(theme::secondary);
    g.drawText(running ? juce::String(juce::roundToInt(processor_.displayBpm.load())) + " BPM" : "Stopped",
               getWidth() - 164, kHeaderHeight + kSuiteHeight + 11, 64, 38, juce::Justification::centredRight);
}

void OpenClapperEditor::resized() {
    processor_.setEditorSize(getWidth(), getHeight());
    auto area = getLocalBounds().reduced(20, 0);
    auto header = area.removeFromTop(kHeaderHeight).withTrimmedTop(30).withHeight(38);
    header.removeFromLeft(280);
    audition_.setBounds(header.removeFromRight(140));
    header.removeFromRight(14);
    next_.setBounds(header.removeFromRight(70));
    header.removeFromRight(5);
    previous_.setBounds(header.removeFromRight(91));
    header.removeFromRight(6);
    presets_.setBounds(header);

    suite_.setBounds(area.removeFromTop(kSuiteHeight).reduced(0, 0));

    auto nav = area.removeFromTop(kNavHeight).reduced(0, 12);
    constexpr std::array widths{82, 95, 95, 160};
    for (size_t i = 0; i < tabs_.size(); ++i) {
        tabs_[i].setBounds(nav.removeFromLeft(widths[i]));
        nav.removeFromLeft(6);
    }
    area.removeFromBottom(14);
    if (page_ == 0) {
        const int wave = juce::jlimit(150, 320, area.getHeight() * 26 / 100);
        waveform_.setBounds(area.removeFromTop(wave));
        area.removeFromTop(8);
    }
    if (page_ >= 0 && page_ < static_cast<int>(pages_.size()) && pages_[static_cast<size_t>(page_)] != nullptr)
        pages_[static_cast<size_t>(page_)]->setBounds(area);

    if (browser_.isVisible()) {
        const int top = kHeaderHeight + kSuiteHeight + 8;
        browser_.setBounds(getWidth() - 450, top, 420, juce::jmin(440, getHeight() - top - 16));
        browser_.toFront(false);
    }
}
