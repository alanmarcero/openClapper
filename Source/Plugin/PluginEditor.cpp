#include "PluginEditor.h"
#include "Presets.h"
#include "BinaryData.h"
#include "Theme.h"

using namespace openclapper;

namespace {
constexpr int kHeaderHeight = 88;
constexpr int kSuiteHeight = 44;
constexpr int kNavHeight = 61;
constexpr int kDesignWidth = 1080;
constexpr int kDesignHeight = 868;
constexpr float kMinScale = 0.75f;
constexpr float kMaxScale = 1.5f;

std::vector<juce::String> factoryNames() {
    std::vector<juce::String> names;
    for (const auto& preset : presets::factoryPresets()) names.emplace_back(preset.name);
    return names;
}
}

OpenClapperEditor::OpenClapperEditor(OpenClapperProcessor& instrument)
    : AudioProcessorEditor(&instrument), processor_(instrument),
      browser_(processor_.userPresets(), factoryNames()), waveform_(instrument),
      hitLight_(instrument.displayTriggerCount.load(), instrument.displayDroppedCount.load()) {
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
    addAndMakeVisible(content_);
    content_.setBounds(0, 0, kDesignWidth, kDesignHeight);
    presets_.setName("Factory preset");
    presets_.setTitle("Preset");
    presets_.setTooltip("Choose a factory clap across six dance styles.");
    presets_.setTextWhenNothingSelected("Custom sound");
    presets_.onChange = [this] {
        if (presets_.getSelectedId() > 0) {
            processor_.selectPreset(presets_.getSelectedId() - 1);
            waveform_.refresh();
            setBrowserVisible(false);
        }
    };
    content_.addAndMakeVisible(presets_);
    content_.addAndMakeVisible(presetLock_);
    previous_.setTooltip("Previous preset");
    next_.setTooltip("Next preset");
    audition_.setTooltip("Play the current sound at A4.");
    previous_.setTitle("Previous preset");
    next_.setTitle("Next preset");
    audition_.setTitle("Audition A4");
    previous_.onClick = [this] { stepPreset(-1); };
    next_.onClick = [this] { stepPreset(1); };
    audition_.onClick = [this] { processor_.triggerAudition(); };
    audition_.setColour(juce::TextButton::buttonColourId, theme::apricot);
    audition_.setColour(juce::TextButton::textColourOffId, theme::plot);
    for (juce::Button* button : { static_cast<juce::Button*>(&previous_),
                                   static_cast<juce::Button*>(&next_),
                                   static_cast<juce::Button*>(&audition_) })
        content_.addAndMakeVisible(*button);
    content_.addAndMakeVisible(hitLight_);
    content_.addAndMakeVisible(meter_);

    suite_.onUndo = [this] { processor_.undo(); refreshChrome(); };
    suite_.onRedo = [this] { processor_.redo(); refreshChrome(); };
    suite_.onShowA = [this] { if (processor_.isAbSlotB()) { processor_.toggleAb(); refreshChrome(); } };
    suite_.onShowB = [this] { if (!processor_.isAbSlotB()) { processor_.toggleAb(); refreshChrome(); } };
    suite_.onCopyAToB = [this] { processor_.copyAbAToB(); refreshChrome(); };
    suite_.onCopyBToA = [this] { processor_.copyAbBToA(); refreshChrome(); };
    suite_.onPresets = [this] { showPresetBrowser(); };
    suite_.onDragWav = [this] {
        auto stem = suite::UserPresetStore::sanitise(processor_.presetName());
        stem = stem.replaceCharacters("\\/:*?\"<>|", "_________");
        if (stem.isEmpty()) stem = "openClapper";
        auto file = juce::File::getSpecialLocation(juce::File::tempDirectory)
            .getChildFile("Alan Marcero").getChildFile("openClapper").getChildFile(stem + ".wav");
        return processor_.renderHitWav(file);
    };
    content_.addAndMakeVisible(suite_);

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
    content_.addChildComponent(browser_);

    constexpr std::array names{ "Clap", "Layers", "Effects", "MIDI & envelope" };
    for (size_t i = 0; i < tabs_.size(); ++i) {
        pages_[i] = std::make_unique<ui::ControlPage>(processor_.parameters, static_cast<ui::Page>(i));
        content_.addChildComponent(*pages_[i]);
        tabs_[i].setComponentID("tab_" + juce::String(i));
        tabs_[i].setButtonText(names[i]);
        tabs_[i].setTitle(names[i]);
        tabs_[i].setTooltip("Open the " + juce::String(names[i]) + " page.");
        tabs_[i].onClick = [this, i] { setPage(static_cast<int>(i)); };
        content_.addAndMakeVisible(tabs_[i]);
    }
    content_.addAndMakeVisible(waveform_);
    setLookAndFeel(&look_);
    setWantsKeyboardFocus(true);
    // Older sessions saved free-aspect sizes. The smaller ratio keeps the whole editor on screen.
    const float savedScale = std::min(static_cast<float>(processor_.editorWidth()) / kDesignWidth,
                                      static_cast<float>(processor_.editorHeight()) / kDesignHeight);
    const float scale = juce::jlimit(kMinScale, kMaxScale, savedScale > 0.0f ? savedScale : 1.0f);
    setResizable(true, true);
    setResizeLimits(juce::roundToInt(kDesignWidth * kMinScale), juce::roundToInt(kDesignHeight * kMinScale),
                    juce::roundToInt(kDesignWidth * kMaxScale), juce::roundToInt(kDesignHeight * kMaxScale));
    getConstrainer()->setFixedAspectRatio(static_cast<double>(kDesignWidth) / kDesignHeight);
    setSize(juce::roundToInt(kDesignWidth * scale), juce::roundToInt(kDesignHeight * scale));
    presets_.setText(processor_.presetDisplayName(), juce::dontSendNotification);
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
    refreshChrome();
}

void OpenClapperEditor::refreshChrome() {
    presets_.setText(processor_.presetDisplayName(), juce::dontSendNotification);
    suite_.setAbSlotB(processor_.isAbSlotB());
    suite_.setUndoEnabled(processor_.canUndo(), processor_.canRedo());
    waveform_.refresh();
    repaint();
}

void OpenClapperEditor::setBrowserVisible(bool visible) {
    browser_.setVisible(visible);
    suite_.setPresetsOpen(visible);
    if (visible) browser_.toFront(false);
}

void OpenClapperEditor::showPresetBrowser() {
    const bool opening = !browser_.isVisible();
    if (opening) {
        browser_.reload();
        const auto user = processor_.userPresetName();
        if (user.isNotEmpty()) {
            browser_.selectPreset(user, false);
        } else if (processor_.getSelectedPresetIndex() >= 0) {
            browser_.selectPreset(processor_.presetName(), true);
        }
    }
    setBrowserVisible(opening);
    resized();
}

void OpenClapperEditor::setPage(int index) {
    setBrowserVisible(false);
    page_ = index;
    for (size_t i = 0; i < tabs_.size(); ++i) {
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
        setBrowserVisible(false);
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
    // setText repaints the box whenever no menu item matches, which is every edited name.
    const auto name = processor_.presetDisplayName();
    if (presets_.getText() != name) presets_.setText(name, juce::dontSendNotification);
    suite_.setAbSlotB(processor_.isAbSlotB());
    suite_.setUndoEnabled(processor_.canUndo(), processor_.canRedo());
    if (page_ == 0 && isShowing()) waveform_.refresh();
    meter_.update(processor_.displayPeak.exchange(0.0f), processor_.displayLimiterGain.exchange(1.0f),
                  ui::outputCeilingDb(processor_.parameters), juce::Time::getMillisecondCounter());
    hitLight_.update(processor_.displayTriggerCount.load(), processor_.displayDroppedCount.load());
    const int handTop = kHeaderHeight + kSuiteHeight;
    content_.repaint(kDesignWidth - 180, handTop, 170, kNavHeight);
}

void OpenClapperEditor::paint(juce::Graphics& g) {
    g.fillAll(theme::paper);
    g.addTransform(content_.getTransform());
    const int chrome = kHeaderHeight + kSuiteHeight;
    g.setColour(theme::surface);
    g.fillRect(0, 0, kDesignWidth, chrome);
    g.setColour(theme::edge);
    g.drawHorizontalLine(chrome - 1, 0, static_cast<float>(kDesignWidth));
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
    g.setFont(juce::FontOptions(30.0f, juce::Font::bold));
    g.drawText("openClapper", 62, 11, 184, 40, juce::Justification::centredLeft);
    g.setColour(theme::secondary);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("v" JucePlugin_VersionString, 248, 20, 48, 24, juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(14.0f));
    g.drawText("Pitched clap synthesizer", 64, 47, 220, 22, juce::Justification::centredLeft);
    g.setColour(theme::ink.withAlpha(0.85f));
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.drawText("Preset", presets_.getX(), 8, 280, 20, juce::Justification::centredLeft);

    const float beat = processor_.displayBeatPhase.load(std::memory_order_relaxed);
    const bool running = beat >= 0.0f;
    const float contact = running ? std::exp(-7.0f * std::min(beat, 1.0f - beat)) : 0.0f;
    const float centre = static_cast<float>(kDesignWidth - 56);
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
               kDesignWidth - 164, kHeaderHeight + kSuiteHeight + 11, 64, 38, juce::Justification::centredRight);
}

void OpenClapperEditor::resized() {
    processor_.setEditorSize(getWidth(), getHeight());
    const float scale = std::min(static_cast<float>(getWidth()) / kDesignWidth,
                                 static_cast<float>(getHeight()) / kDesignHeight);
    content_.setTransform(juce::AffineTransform::scale(scale).translated(
        (static_cast<float>(getWidth()) - kDesignWidth * scale) * 0.5f,
        (static_cast<float>(getHeight()) - kDesignHeight * scale) * 0.5f));
    auto area = content_.getLocalBounds().reduced(20, 0);
    auto header = area.removeFromTop(kHeaderHeight).withTrimmedTop(30).withHeight(38);
    header.removeFromLeft(280);
    audition_.setBounds(header.removeFromRight(140));
    header.removeFromRight(6);
    hitLight_.setBounds(header.removeFromRight(40));
    header.removeFromRight(6);
    next_.setBounds(header.removeFromRight(40).withSizeKeepingCentre(40, 40));
    header.removeFromRight(5);
    previous_.setBounds(header.removeFromRight(40).withSizeKeepingCentre(40, 40));
    header.removeFromRight(6);
    presetLock_.setBounds(header.removeFromRight(24).withSizeKeepingCentre(22, 22));
    header.removeFromRight(4);
    presets_.setBounds(header);

    suite_.setBounds(area.removeFromTop(kSuiteHeight).reduced(0, 0));

    auto nav = area.removeFromTop(kNavHeight).reduced(0, 12);
    constexpr std::array widths{82, 95, 95, 160};
    for (size_t i = 0; i < tabs_.size(); ++i) {
        tabs_[i].setBounds(nav.removeFromLeft(widths[i]));
        nav.removeFromLeft(6);
    }
    meter_.setBounds(nav.getX() + 24, nav.getY(), 380, nav.getHeight());
    area.removeFromBottom(14);
    if (page_ == 0) {
        waveform_.setBounds(area.removeFromTop(165));
        area.removeFromTop(8);
    }
    if (page_ >= 0 && page_ < static_cast<int>(pages_.size()) && pages_[static_cast<size_t>(page_)] != nullptr)
        pages_[static_cast<size_t>(page_)]->setBounds(area);

    if (browser_.isVisible()) {
        const int top = kHeaderHeight + kSuiteHeight + 8;
        browser_.setBounds(kDesignWidth - 450, top, 420, juce::jmin(440, kDesignHeight - top - 16));
        browser_.toFront(false);
    }
}
