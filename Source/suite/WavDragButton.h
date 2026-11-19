#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace suite {

/** Drag handle that writes a file on drag and hands it to the host or desktop. */
class WavDragButton : public juce::TextButton {
public:
    WavDragButton() : juce::TextButton("Drag WAV") {
        setTooltip("Drag the current hit out as a WAV file.");
    }

    std::function<juce::File()> onPrepareFile;

    [[nodiscard]] juce::File exportFile() const { return onPrepareFile ? onPrepareFile() : juce::File(); }

    void mouseDown(const juce::MouseEvent& event) override {
        dragStarted_ = false;
        juce::TextButton::mouseDown(event);
    }

    void mouseDrag(const juce::MouseEvent& event) override {
        if (dragStarted_ || event.getDistanceFromDragStart() < 8) return;
        dragStarted_ = true;
        const auto file = exportFile();
        if (!file.existsAsFile()) return;
        juce::StringArray files;
        files.add(file.getFullPathName());
        juce::DragAndDropContainer::performExternalDragDropOfFiles(files, false, this);
    }

private:
    bool dragStarted_ = false;
};

} // namespace suite
