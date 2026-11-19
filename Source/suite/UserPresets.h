#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include <optional>
#include <vector>

namespace suite {

struct PresetEntry {
    juce::String name;
    bool factory = false;
    bool favorite = false;
    int factoryIndex = -1;
};

/** User presets and favorites stored under ~/Documents/Alan Marcero/<plugin>/Presets. */
class UserPresetStore {
public:
    explicit UserPresetStore(juce::File directory);

    static juce::File defaultDirectory(const juce::String& pluginName);
    static juce::String sanitise(juce::String name);

    void setDirectory(juce::File directory) { directory_ = std::move(directory); }
    [[nodiscard]] const juce::File& directory() const noexcept { return directory_; }

    [[nodiscard]] std::vector<juce::String> userNames() const;
    bool save(const juce::String& name, const juce::ValueTree& parameters);
    [[nodiscard]] std::optional<juce::ValueTree> load(const juce::String& name) const;
    bool rename(const juce::String& from, const juce::String& to);
    bool remove(const juce::String& name);

    [[nodiscard]] bool isFavorite(const juce::String& name, bool factory) const;
    void setFavorite(const juce::String& name, bool factory, bool favorite);

private:
    [[nodiscard]] juce::File fileFor(const juce::String& name) const;
    [[nodiscard]] std::vector<juce::String> readFavoriteKeys() const;
    void writeFavoriteKeys(const std::vector<juce::String>& keys) const;
    static juce::String favoriteKey(const juce::String& name, bool factory);

    juce::File directory_;
};

} // namespace suite
