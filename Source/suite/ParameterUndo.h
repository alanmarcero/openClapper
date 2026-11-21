#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <algorithm>
#include <array>
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

    [[nodiscard]] juce::UndoManager& manager() noexcept { return histories_[active_]; }
    [[nodiscard]] bool canUndo() const { return histories_[active_].canUndo(); }
    [[nodiscard]] bool canRedo() const { return histories_[active_].canRedo(); }
    void undo() { histories_[active_].undo(); }
    void redo() { histories_[active_].redo(); }
    void clear() { std::ranges::for_each(histories_, [](juce::UndoManager& history) { history.clearUndoHistory(); }); }

    // Each A/B slot has its own history. Edits, undo and redo use the selected one.
    void selectHistory(bool slotB) noexcept { active_ = slotB ? 1 : 0; }
    void clearHistory(bool slotB) { histories_[slotB ? 1 : 0].clearUndoHistory(); }

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
    std::array<juce::UndoManager, 2> histories_;
    size_t active_ = 0;
    Capture capture_;
    Restore restore_;
    juce::ValueTree gestureStart_;
    int depth_ = 0;
    int suspended_ = 0;
    bool applying_ = false;
};

} // namespace suite
