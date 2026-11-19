#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace suite {

/** Two full plugin-state slots for A/B compare. */
class AbSlots {
public:
    void setBoth(const juce::ValueTree& state) {
        a_ = state.createCopy();
        b_ = state.createCopy();
        editingB_ = false;
    }

    void store(bool slotB, const juce::ValueTree& state) {
        (slotB ? b_ : a_) = state.createCopy();
    }

    [[nodiscard]] const juce::ValueTree& slot(bool slotB) const { return slotB ? b_ : a_; }
    [[nodiscard]] bool editingB() const noexcept { return editingB_; }
    void setEditingB(bool value) noexcept { editingB_ = value; }
    [[nodiscard]] bool hasSlots() const noexcept { return a_.isValid() && b_.isValid(); }

    void copyAToB() { if (a_.isValid()) b_ = a_.createCopy(); }
    void copyBToA() { if (b_.isValid()) a_ = b_.createCopy(); }

    void write(juce::ValueTree& parent) const {
        parent.removeChild(parent.getChildWithName("ABCompare"), nullptr);
        if (!hasSlots()) return;
        juce::ValueTree node("ABCompare");
        node.setProperty("active", editingB_ ? "B" : "A", nullptr);
        node.appendChild(a_.createCopy(), nullptr);
        node.appendChild(b_.createCopy(), nullptr);
        parent.appendChild(node, nullptr);
    }

    bool read(const juce::ValueTree& parent) {
        const auto node = parent.getChildWithName("ABCompare");
        if (!node.isValid() || node.getNumChildren() < 2) return false;
        a_ = node.getChild(0).createCopy();
        b_ = node.getChild(1).createCopy();
        editingB_ = node.getProperty("active").toString() == "B";
        return a_.isValid() && b_.isValid();
    }

private:
    juce::ValueTree a_, b_;
    bool editingB_ = false;
};

} // namespace suite
