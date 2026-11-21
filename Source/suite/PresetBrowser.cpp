#include "PresetBrowser.h"

namespace suite {
namespace {
constexpr int kConfirmMs = 3000;

class PresetRow : public juce::Component {
public:
    std::function<void()> onStar;

    void setContent(const PresetEntry& entry, SuitePalette palette, bool selected) {
        entry_ = entry;
        palette_ = palette;
        selected_ = selected;
        repaint();
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(selected_ ? palette_.selected : palette_.surface);
        g.setColour(entry_.favorite ? palette_.accent : palette_.secondary);
        g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        g.drawText(entry_.favorite ? "*" : "+", 4, 0, 28, getHeight(), juce::Justification::centred);
        g.setColour(palette_.ink);
        g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        auto text = entry_.name;
        if (!entry_.factory) text = "User  " + text;
        g.drawText(text, 36, 0, getWidth() - 42, getHeight(), juce::Justification::centredLeft, true);
    }

    // Only the star takes clicks. The ListBox gets the rest, so it can select and load the row.
    bool hitTest(int x, int) override { return x < 32; }

    void mouseDown(const juce::MouseEvent&) override {
        if (onStar) onStar();
    }

private:
    PresetEntry entry_;
    SuitePalette palette_;
    bool selected_ = false;
};

} // namespace

PresetBrowser::PresetBrowser(UserPresetStore& store, std::vector<juce::String> factoryNames)
    : store_(store), factoryNames_(std::move(factoryNames)) {
    search_.setName("Preset search");
    search_.setTooltip("Search factory and user presets.");
    search_.setTextToShowWhenEmpty("Search presets", palette_.secondary);
    search_.setColour(juce::TextEditor::backgroundColourId, palette_.paper);
    search_.setColour(juce::TextEditor::textColourId, palette_.ink);
    search_.setColour(juce::TextEditor::outlineColourId, palette_.edge);
    search_.onTextChange = [this] { applyFilter(); };
    addAndMakeVisible(search_);

    name_.setName("Preset name");
    name_.setTooltip("Name used when saving or renaming a user preset.");
    name_.setTextToShowWhenEmpty("Preset name", palette_.secondary);
    name_.setColour(juce::TextEditor::backgroundColourId, palette_.paper);
    name_.setColour(juce::TextEditor::textColourId, palette_.ink);
    name_.setColour(juce::TextEditor::outlineColourId, palette_.edge);
    name_.onTextChange = [this] { disarm(); };
    addAndMakeVisible(name_);

    save_.setTooltip("Save the current sound as a user preset. Factory presets stay unchanged.");
    rename_.setTooltip("Rename the selected user preset.");
    remove_.setTooltip("Delete the selected user preset.");
    // Replacing or deleting a user preset takes a second click within 3 seconds.
    save_.onClick = [this] {
        if (armed_ != &save_ && store_.load(name_.getText()).has_value()) {
            disarm();
            armed_ = &save_;
            save_.setButtonText("Replace?");
            startTimer(kConfirmMs);
            return;
        }
        disarm();
        if (onSave && onSave(name_.getText())) reload();
    };
    rename_.onClick = [this] {
        if (selectedRow_ < 0 || selectedRow_ >= visibleCount()) return;
        const auto& entry = visible_[static_cast<size_t>(selectedRow_)];
        if (entry.factory || !onRename) return;
        if (onRename(entry.name, name_.getText())) reload();
    };
    remove_.onClick = [this] {
        if (selectedRow_ < 0 || selectedRow_ >= visibleCount()) return;
        const auto& entry = visible_[static_cast<size_t>(selectedRow_)];
        if (entry.factory || !onDelete) return;
        if (armed_ != &remove_) {
            disarm();
            armed_ = &remove_;
            remove_.setButtonText("Delete?");
            startTimer(kConfirmMs);
            return;
        }
        disarm();
        if (onDelete(entry.name)) {
            selectedRow_ = -1;
            reload();
        }
    };
    for (auto* button : { &save_, &rename_, &remove_ }) addAndMakeVisible(*button);

    list_.setRowHeight(36);
    list_.setModel(this);
    list_.setColour(juce::ListBox::backgroundColourId, palette_.surface);
    list_.setColour(juce::ListBox::outlineColourId, palette_.edge);
    addAndMakeVisible(list_);
    reload();
}

PresetBrowser::~PresetBrowser() { list_.setModel(nullptr); }

void PresetBrowser::disarm() {
    stopTimer();
    armed_ = nullptr;
    save_.setButtonText("Save");
    remove_.setButtonText("Delete");
}

void PresetBrowser::setPalette(SuitePalette palette) {
    palette_ = palette;
    search_.setColour(juce::TextEditor::backgroundColourId, palette_.paper);
    search_.setColour(juce::TextEditor::textColourId, palette_.ink);
    search_.setColour(juce::TextEditor::outlineColourId, palette_.edge);
    search_.applyColourToAllText(palette_.ink);
    name_.setColour(juce::TextEditor::backgroundColourId, palette_.paper);
    name_.setColour(juce::TextEditor::textColourId, palette_.ink);
    name_.setColour(juce::TextEditor::outlineColourId, palette_.edge);
    name_.applyColourToAllText(palette_.ink);
    list_.setColour(juce::ListBox::backgroundColourId, palette_.surface);
    list_.setColour(juce::ListBox::outlineColourId, palette_.edge);
    repaint();
    list_.updateContent();
}

void PresetBrowser::reload() {
    const auto selected = selectedRow_ >= 0 && selectedRow_ < visibleCount()
        ? visible_[static_cast<size_t>(selectedRow_)].name : juce::String();
    const bool selectedFactory = selectedRow_ >= 0 && selectedRow_ < visibleCount()
        && visible_[static_cast<size_t>(selectedRow_)].factory;
    all_.clear();
    for (size_t index = 0; index < factoryNames_.size(); ++index) {
        const auto& name = factoryNames_[index];
        all_.push_back({ name, true, store_.isFavorite(name, true), static_cast<int>(index) });
    }
    for (const auto& name : store_.userNames())
        all_.push_back({ name, false, store_.isFavorite(name, false), -1 });
    applyFilter();
    selectedRow_ = -1;
    for (int row = 0; row < visibleCount(); ++row) {
        const auto& entry = visible_[static_cast<size_t>(row)];
        if (entry.name == selected && entry.factory == selectedFactory) selectedRow_ = row;
    }
    if (selectedRow_ >= 0) list_.selectRow(selectedRow_);
    updateActions();
}

void PresetBrowser::setSearchText(const juce::String& text) {
    search_.setText(text, false);
    applyFilter();
}

juce::String PresetBrowser::rowName(int index) const {
    if (index < 0 || index >= visibleCount()) return {};
    return visible_[static_cast<size_t>(index)].name;
}

void PresetBrowser::paint(juce::Graphics& g) {
    g.setColour(palette_.surface);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);
    g.setColour(palette_.edge);
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 8.0f, 1.0f);
    g.setColour(palette_.secondary);
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawText("Presets", 12, 8, 120, 18, juce::Justification::centredLeft);
}

void PresetBrowser::resized() {
    auto area = getLocalBounds().reduced(10, 8);
    area.removeFromTop(22);
    search_.setBounds(area.removeFromTop(32));
    area.removeFromTop(6);
    auto footer = area.removeFromBottom(76);
    name_.setBounds(footer.removeFromTop(32));
    footer.removeFromTop(6);
    const int width = footer.getWidth() / 3;
    save_.setBounds(footer.removeFromLeft(width).reduced(2, 0));
    rename_.setBounds(footer.removeFromLeft(width).reduced(2, 0));
    remove_.setBounds(footer.reduced(2, 0));
    list_.setBounds(area.reduced(0, 4));
}

void PresetBrowser::applyFilter() {
    const auto query = search_.getText().trim();
    visible_.clear();
    for (const auto& entry : all_)
        if (query.isEmpty() || entry.name.containsIgnoreCase(query)) visible_.push_back(entry);
    selectedRow_ = -1;
    list_.updateContent();
    list_.repaint();
    updateActions();
}

void PresetBrowser::updateActions() {
    const bool user = selectedRow_ >= 0 && selectedRow_ < visibleCount()
        && !visible_[static_cast<size_t>(selectedRow_)].factory;
    rename_.setEnabled(user);
    remove_.setEnabled(user);
}

void PresetBrowser::choose(int row, bool favoriteOnly) {
    if (row < 0 || row >= visibleCount()) return;
    auto& entry = visible_[static_cast<size_t>(row)];
    if (favoriteOnly) {
        entry.favorite = !entry.favorite;
        store_.setFavorite(entry.name, entry.factory, entry.favorite);
        for (auto& stored : all_)
            if (stored.factory == entry.factory && stored.name == entry.name) stored.favorite = entry.favorite;
        list_.updateContent();
        return;
    }
    selectedRow_ = row;
    list_.selectRow(row);
    name_.setText(entry.name, juce::dontSendNotification);
    updateActions();
    if (entry.factory) {
        if (onFactoryPreset) onFactoryPreset(entry.factoryIndex);
        return;
    }
    if (onUserPreset) onUserPreset(entry.name);
}

juce::Component* PresetBrowser::refreshComponentForRow(int row, bool selected, juce::Component* existing) {
    if (row < 0 || row >= visibleCount()) return nullptr;
    auto* line = dynamic_cast<PresetRow*>(existing);
    if (line == nullptr) line = new PresetRow();
    const auto entry = visible_[static_cast<size_t>(row)];
    line->setContent(entry, palette_, selected || row == selectedRow_);
    line->onStar = [this, row] { choose(row, true); };
    return line;
}

void PresetBrowser::listBoxItemClicked(int row, const juce::MouseEvent&) { choose(row, false); }

void PresetBrowser::listBoxItemDoubleClicked(int, const juce::MouseEvent&) { setVisible(false); }

void PresetBrowser::returnKeyPressed(int row) {
    choose(row, false);
    setVisible(false);
}

void PresetBrowser::selectedRowsChanged(int lastRow) {
    disarm();
    if (lastRow >= 0 && lastRow < visibleCount()) {
        selectedRow_ = lastRow;
        name_.setText(visible_[static_cast<size_t>(lastRow)].name, juce::dontSendNotification);
    }
    updateActions();
}

} // namespace suite
