#pragma once

#include "UserPresets.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace suite {

struct SuitePalette {
    juce::Colour paper { 0xff141414 };
    juce::Colour surface { 0xff1c1c1c };
    juce::Colour ink { 0xfff2f2f2 };
    juce::Colour secondary { 0xff9aa0a6 };
    juce::Colour edge { 0xff333333 };
    juce::Colour accent { 0xff48f0c8 };
    juce::Colour selected { 0xff1f4842 };
};

/** Factory and user presets with text search and favorites. Factory rows stay read-only. */
class PresetBrowser : public juce::Component, private juce::ListBoxModel {
public:
    PresetBrowser(UserPresetStore& store, std::vector<juce::String> factoryNames);

    void setPalette(SuitePalette palette);
    void reload();
    void setSearchText(const juce::String& text);

    std::function<void(int factoryIndex)> onFactoryPreset;
    std::function<void(const juce::String& name)> onUserPreset;
    std::function<bool(const juce::String& name)> onSave;
    std::function<bool(const juce::String& from, const juce::String& to)> onRename;
    std::function<bool(const juce::String& name)> onDelete;

    [[nodiscard]] int visibleCount() const { return static_cast<int>(visible_.size()); }
    [[nodiscard]] juce::String rowName(int index) const;
    [[nodiscard]] juce::TextEditor& search() noexcept { return search_; }
    [[nodiscard]] juce::TextEditor& nameEditor() noexcept { return name_; }

    ~PresetBrowser() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    int getNumRows() override { return visibleCount(); }
    void paintListBoxItem(int, juce::Graphics&, int, int, bool) override {}
    juce::Component* refreshComponentForRow(int row, bool selected, juce::Component* existing) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;
    void returnKeyPressed(int row) override;
    void selectedRowsChanged(int lastRow) override;

    void applyFilter();
    void updateActions();
    void choose(int row, bool favoriteOnly);

    UserPresetStore& store_;
    std::vector<juce::String> factoryNames_;
    std::vector<PresetEntry> all_;
    std::vector<PresetEntry> visible_;
    SuitePalette palette_;
    juce::TextEditor search_, name_;
    juce::TextButton save_{ "Save" }, rename_{ "Rename" }, remove_{ "Delete" };
    juce::ListBox list_{ "Presets", this };
    int selectedRow_ = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowser)
};

} // namespace suite
