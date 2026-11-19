#include "SuiteBar.h"

namespace suite {

SuiteBar::SuiteBar() {
    undo_.setTooltip("Undo the last parameter change. Shortcut: Cmd/Ctrl+Z.");
    redo_.setTooltip("Redo the last undone change. Shortcut: Shift+Cmd/Ctrl+Z.");
    a_.setTooltip("Hear slot A.");
    b_.setTooltip("Hear slot B.");
    aToB_.setTooltip("Copy slot A onto slot B.");
    bToA_.setTooltip("Copy slot B onto slot A.");
    presets_.setTooltip("Browse factory and user presets.");
    a_.setClickingTogglesState(false);
    b_.setClickingTogglesState(false);
    undo_.onClick = [this] { if (onUndo) onUndo(); };
    redo_.onClick = [this] { if (onRedo) onRedo(); };
    a_.onClick = [this] { if (onShowA) onShowA(); };
    b_.onClick = [this] { if (onShowB) onShowB(); };
    aToB_.onClick = [this] { if (onCopyAToB) onCopyAToB(); };
    bToA_.onClick = [this] { if (onCopyBToA) onCopyBToA(); };
    presets_.onClick = [this] { if (onPresets) onPresets(); };
    drag_.onPrepareFile = [this] { return onDragWav ? onDragWav() : juce::File(); };
    for (auto* button : { &undo_, &redo_, &a_, &b_, &aToB_, &bToA_, &presets_, static_cast<juce::TextButton*>(&drag_) })
        addAndMakeVisible(*button);
    setAbSlotB(false);
    setUndoEnabled(false, false);
}

void SuiteBar::setAbSlotB(bool editingB) {
    a_.setToggleState(!editingB, juce::dontSendNotification);
    b_.setToggleState(editingB, juce::dontSendNotification);
}

void SuiteBar::setUndoEnabled(bool canUndo, bool canRedo) {
    undo_.setEnabled(canUndo);
    redo_.setEnabled(canRedo);
}

void SuiteBar::resized() {
    auto area = getLocalBounds();
    auto takeLeft = [&](juce::Component& component, int width) {
        component.setBounds(area.removeFromLeft(width).reduced(0, 4));
        area.removeFromLeft(6);
    };
    takeLeft(undo_, 72);
    takeLeft(redo_, 72);
    area.removeFromLeft(8);
    takeLeft(a_, 48);
    takeLeft(b_, 48);
    area.removeFromLeft(8);
    takeLeft(aToB_, 78);
    takeLeft(bToA_, 78);
    drag_.setBounds(area.removeFromRight(112).reduced(0, 4));
    area.removeFromRight(6);
    presets_.setBounds(area.removeFromRight(96).reduced(0, 4));
}

} // namespace suite
