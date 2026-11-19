#include "UserPresets.h"
#include <algorithm>

namespace suite {

UserPresetStore::UserPresetStore(juce::File directory) : directory_(std::move(directory)) {}

juce::File UserPresetStore::defaultDirectory(const juce::String& pluginName) {
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("Alan Marcero")
        .getChildFile(pluginName)
        .getChildFile("Presets");
}

juce::String UserPresetStore::sanitise(juce::String name) {
    name = name.trim();
    if (name.isEmpty() || name == "." || name == "..") return {};
    return name.substring(0, 80);
}

juce::String UserPresetStore::favoriteKey(const juce::String& name, bool factory) {
    return (factory ? "factory\n" : "user\n") + name;
}

juce::File UserPresetStore::fileFor(const juce::String& name) const {
    auto stem = name;
    stem = stem.replaceCharacters("\\/:*?\"<>|", "_________");
    return directory_.getChildFile(stem + ".xml");
}

std::vector<juce::String> UserPresetStore::userNames() const {
    std::vector<juce::String> names;
    if (!directory_.isDirectory()) return names;
    for (const auto& file : directory_.findChildFiles(juce::File::findFiles, false, "*.xml")) {
        if (file.getFileName() == "favorites.xml") continue;
        const auto xml = juce::XmlDocument::parse(file);
        if (xml == nullptr || !xml->hasTagName("OpenClapperUserPreset")) continue;
        auto name = sanitise(xml->getStringAttribute("name", file.getFileNameWithoutExtension()));
        if (name.isNotEmpty()) names.push_back(name);
    }
    std::sort(names.begin(), names.end(), [](const juce::String& a, const juce::String& b) {
        return a.compareIgnoreCase(b) < 0;
    });
    return names;
}

bool UserPresetStore::save(const juce::String& name, const juce::ValueTree& parameters) {
    const auto clean = sanitise(name);
    if (clean.isEmpty() || !parameters.isValid()) return false;
    directory_.createDirectory();
    juce::XmlElement root("OpenClapperUserPreset");
    root.setAttribute("name", clean);
    if (auto xml = parameters.createXml()) root.addChildElement(xml.release());
    return root.writeTo(fileFor(clean));
}

std::optional<juce::ValueTree> UserPresetStore::load(const juce::String& name) const {
    const auto clean = sanitise(name);
    if (clean.isEmpty()) return std::nullopt;
    const auto xml = juce::XmlDocument::parse(fileFor(clean));
    if (xml == nullptr || !xml->hasTagName("OpenClapperUserPreset")) return std::nullopt;
    const auto* parameters = xml->getChildByName("PARAMETERS");
    if (parameters == nullptr) return std::nullopt;
    auto tree = juce::ValueTree::fromXml(*parameters);
    if (!tree.isValid()) return std::nullopt;
    return tree;
}

bool UserPresetStore::rename(const juce::String& from, const juce::String& to) {
    const auto source = sanitise(from);
    const auto target = sanitise(to);
    if (source.isEmpty() || target.isEmpty()) return false;
    auto loaded = load(source);
    if (!loaded) return false;
    if (source == target) return true;
    if (!save(target, *loaded)) return false;
    if (fileFor(source) != fileFor(target)) fileFor(source).deleteFile();
    auto keys = readFavoriteKeys();
    const auto oldKey = favoriteKey(source, false);
    const auto newKey = favoriteKey(target, false);
    bool changed = false;
    for (auto& key : keys) {
        if (key == oldKey) {
            key = newKey;
            changed = true;
        }
    }
    if (changed) writeFavoriteKeys(keys);
    return true;
}

bool UserPresetStore::remove(const juce::String& name) {
    const auto clean = sanitise(name);
    if (clean.isEmpty()) return false;
    const auto file = fileFor(clean);
    if (!file.existsAsFile()) return false;
    const bool deleted = file.deleteFile();
    auto keys = readFavoriteKeys();
    const auto key = favoriteKey(clean, false);
    const auto before = keys.size();
    std::erase_if(keys, [&](const juce::String& item) { return item == key; });
    if (keys.size() != before) writeFavoriteKeys(keys);
    return deleted;
}

std::vector<juce::String> UserPresetStore::readFavoriteKeys() const {
    std::vector<juce::String> keys;
    const auto xml = juce::XmlDocument::parse(directory_.getChildFile("favorites.xml"));
    if (xml == nullptr) return keys;
    for (auto* item = xml->getFirstChildElement(); item != nullptr; item = item->getNextElement()) {
        if (!item->hasTagName("Item")) continue;
        const auto name = item->getStringAttribute("name");
        if (name.isEmpty()) continue;
        keys.push_back(favoriteKey(name, item->getStringAttribute("kind") == "factory"));
    }
    return keys;
}

void UserPresetStore::writeFavoriteKeys(const std::vector<juce::String>& keys) const {
    directory_.createDirectory();
    juce::XmlElement root("Favorites");
    for (const auto& key : keys) {
        auto* item = root.createNewChildElement("Item");
        const bool factory = key.startsWith("factory\n");
        item->setAttribute("kind", factory ? "factory" : "user");
        item->setAttribute("name", key.fromFirstOccurrenceOf("\n", false, false));
    }
    root.writeTo(directory_.getChildFile("favorites.xml"));
}

bool UserPresetStore::isFavorite(const juce::String& name, bool factory) const {
    const auto key = favoriteKey(name, factory);
    const auto keys = readFavoriteKeys();
    return std::find(keys.begin(), keys.end(), key) != keys.end();
}

void UserPresetStore::setFavorite(const juce::String& name, bool factory, bool favorite) {
    const auto clean = sanitise(name);
    if (clean.isEmpty()) return;
    auto keys = readFavoriteKeys();
    const auto key = favoriteKey(clean, factory);
    std::erase_if(keys, [&](const juce::String& item) { return item == key; });
    if (favorite) keys.push_back(key);
    writeFavoriteKeys(keys);
}

} // namespace suite
