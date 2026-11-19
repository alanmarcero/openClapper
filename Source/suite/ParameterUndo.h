#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>

namespace suite {

/** Groups parameter edits into single UndoManager steps.
    A slider drag, button press, or explicit group() call becomes one undo step.
    Host updates that never open a gesture are left out of the history.
*/
class ParameterUndo : private juce::AudioProcessorParameter::Listener {
public:
    ParameterUndo() = default;
    ~ParameterUndo() override;

    void attach(juce::AudioProcessor& processor);
    void detach();

    using Capture = std::function<juce::ValueTree()>;
    using Restore = std::function<void(const juce::ValueTree&)>;
    void setStateAccess(Capture capture, Restore restore);

    [[nodiscard]] juce::UndoManager& manager() noexcept { return undo_; }
    [[nodiscard]] bool canUndo() const { return undo_.canUndo(); }
    [[nodiscard]] bool canRedo() const { return undo_.canRedo(); }
    void undo() { undo_.undo(); }
    void redo() { undo_.redo(); }
    void clear() { undo_.clearUndoHistory(); }

    void suspend() { ++suspended_; }
    void resume() { if (suspended_ > 0) --suspended_; }

    void group(const juce::String& name, const std::function<void()>& edit);

private:
    struct Swap;

    void parameterValueChanged(int, float) override {}
    void parameterGestureChanged(int, bool starting) override;
    void commit(juce::ValueTree before, juce::ValueTree after, const juce::String& name);
    void apply(const juce::ValueTree& tree);

    juce::AudioProcessor* processor_ = nullptr;
    juce::UndoManager undo_;
    Capture capture_;
    Restore restore_;
    juce::ValueTree gestureStart_;
    int depth_ = 0;
    int suspended_ = 0;
    bool applying_ = false;
};

} // namespace suite
