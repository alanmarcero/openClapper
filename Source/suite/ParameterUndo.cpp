#include "ParameterUndo.h"

namespace suite {

struct ParameterUndo::Swap : juce::UndoableAction {
    ParameterUndo& owner;
    juce::ValueTree before, after;

    Swap(ParameterUndo& undo, juce::ValueTree previous, juce::ValueTree next)
        : owner(undo), before(std::move(previous)), after(std::move(next)) {}

    bool perform() override { owner.apply(after); return true; }
    bool undo() override { owner.apply(before); return true; }
    int getSizeInUnits() override { return 8; }
};

ParameterUndo::~ParameterUndo() { detach(); }

void ParameterUndo::attach(juce::AudioProcessor& processor) {
    detach();
    processor_ = &processor;
    for (auto* parameter : processor_->getParameters())
        parameter->addListener(this);
}

void ParameterUndo::detach() {
    if (processor_ == nullptr) return;
    for (auto* parameter : processor_->getParameters())
        parameter->removeListener(this);
    processor_ = nullptr;
}

void ParameterUndo::setStateAccess(Capture capture, Restore restore) {
    capture_ = std::move(capture);
    restore_ = std::move(restore);
}

void ParameterUndo::group(const juce::String& name, const std::function<void()>& edit) {
    if (suspended_ > 0 || !capture_) {
        edit();
        return;
    }
    auto before = capture_();
    {
        juce::ScopedValueSetter<int> guard(suspended_, suspended_ + 1);
        edit();
    }
    commit(std::move(before), capture_(), name);
}

void ParameterUndo::parameterGestureChanged(int, bool starting) {
    if (processor_ == nullptr || applying_ || suspended_ > 0 || !capture_) return;
    if (auto* messages = juce::MessageManager::getInstanceWithoutCreating())
        if (!messages->isThisTheMessageThread()) return;

    if (starting) {
        if (depth_++ == 0) gestureStart_ = capture_();
        return;
    }
    if (depth_ == 0) return;
    if (--depth_ > 0) return;
    commit(std::move(gestureStart_), capture_(), "Edit");
}

void ParameterUndo::commit(juce::ValueTree before, juce::ValueTree after, const juce::String& name) {
    if (!before.isValid() || before.isEquivalentTo(after)) return;
    undo_.beginNewTransaction(name);
    undo_.perform(new Swap(*this, std::move(before), std::move(after)), name);
}

void ParameterUndo::apply(const juce::ValueTree& tree) {
    if (!restore_) return;
    juce::ScopedValueSetter<bool> guard(applying_, true);
    restore_(tree.createCopy());
}

} // namespace suite
