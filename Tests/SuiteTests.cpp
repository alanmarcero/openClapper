#include "Parameters.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Presets.h"
#include "suite/PresetBrowser.h"
#include "suite/WavDragButton.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstring>

namespace {
juce::File tempDirectory(const char* name) {
    auto directory = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("openclapper-suite-tests")
        .getChildFile(name);
    directory.deleteRecursively();
    directory.createDirectory();
    return directory;
}

template <typename Widget, typename Predicate>
Widget* findVisible(juce::Component& root, Predicate matches) {
    for (auto* child : root.getChildren()) {
        if (!child->isVisible()) continue;
        if (auto* widget = dynamic_cast<Widget*>(child); widget && matches(*widget)) return widget;
        if (auto* result = findVisible<Widget>(*child, matches)) return result;
    }
    return nullptr;
}

float rawValue(OpenClapperProcessor& processor, const char* id) {
    return processor.parameters.getRawParameterValue(id)->load();
}

void gesture(OpenClapperProcessor& processor, const char* id, float value) {
    auto* parameter = processor.parameters.getParameter(id);
    REQUIRE(parameter != nullptr);
    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    parameter->endChangeGesture();
}

// Presses and releases the mouse on the component that the point hits, as a real click does.
void clickAt(juce::Component& root, juce::Point<int> point) {
    auto* target = root.getComponentAt(point);
    REQUIRE(target != nullptr);
    const auto local = target->getLocalPoint(&root, point).toFloat();
    const auto now = juce::Time::getCurrentTime();
    const juce::MouseEvent press(juce::Desktop::getInstance().getMainMouseSource(), local,
        juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier), juce::MouseInputSource::defaultPressure,
        juce::MouseInputSource::defaultOrientation, juce::MouseInputSource::defaultRotation,
        juce::MouseInputSource::defaultTiltX, juce::MouseInputSource::defaultTiltY,
        target, target, now, local, now, 1, false);
    target->mouseDown(press);
    target->mouseUp(press);
}

bool samePixels(const juce::Image& first, const juce::Image& second) {
    const juce::Image::BitmapData a(first, juce::Image::BitmapData::readOnly);
    const juce::Image::BitmapData b(second, juce::Image::BitmapData::readOnly);
    return a.size == b.size && std::memcmp(a.data, b.data, a.size) == 0;
}
}

TEST_CASE("Parameter gestures undo and redo as one step", "[plugin][suite]") {
    OpenClapperProcessor processor;
    const float original = rawValue(processor, openclapper::params::kToneDecay);
    auto* parameter = processor.parameters.getParameter(openclapper::params::kToneDecay);
    parameter->setValueNotifyingHost(parameter->convertTo0to1(240.0f));
    REQUIRE_FALSE(processor.canUndo());

    parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(parameter->convertTo0to1(80.0f));
    parameter->setValueNotifyingHost(parameter->convertTo0to1(320.0f));
    parameter->endChangeGesture();
    REQUIRE(rawValue(processor, openclapper::params::kToneDecay) == Catch::Approx(320.0f));
    REQUIRE(processor.canUndo());
    REQUIRE_FALSE(processor.canRedo());

    const float spreadBefore = rawValue(processor, openclapper::params::kPreSpread);
    gesture(processor, openclapper::params::kPreSpread, 4.0f);
    processor.undo();
    REQUIRE(rawValue(processor, openclapper::params::kPreSpread) == Catch::Approx(spreadBefore));
    REQUIRE(rawValue(processor, openclapper::params::kToneDecay) == Catch::Approx(320.0f));
    processor.undo();
    REQUIRE(rawValue(processor, openclapper::params::kToneDecay) == Catch::Approx(240.0f));
    REQUIRE(rawValue(processor, openclapper::params::kToneDecay) != Catch::Approx(original));
    REQUIRE(processor.canRedo());
    processor.redo();
    REQUIRE(rawValue(processor, openclapper::params::kToneDecay) == Catch::Approx(320.0f));
    processor.redo();
    REQUIRE(rawValue(processor, openclapper::params::kPreSpread) == Catch::Approx(4.0f));
}

TEST_CASE("Loading a factory preset is a single undo step", "[plugin][suite]") {
    OpenClapperProcessor processor;
    gesture(processor, openclapper::params::kNoiseDecay, 900.0f);
    const float tweaked = rawValue(processor, openclapper::params::kNoiseDecay);
    processor.selectPreset(4);
    REQUIRE(processor.getSelectedPresetIndex() == 4);
    REQUIRE(rawValue(processor, openclapper::params::kNoiseDecay) != Catch::Approx(tweaked));
    processor.undo();
    REQUIRE(processor.getSelectedPresetIndex() == 0);
    REQUIRE(rawValue(processor, openclapper::params::kNoiseDecay) == Catch::Approx(tweaked));
    processor.redo();
    REQUIRE(processor.getSelectedPresetIndex() == 4);
}

TEST_CASE("Editor shortcuts and buttons undo parameter changes", "[plugin][suite][gui]") {
    OpenClapperProcessor processor;
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    editor->setVisible(true);
    const float levelBefore = rawValue(processor, openclapper::params::kToneLevel);
    gesture(processor, openclapper::params::kToneLevel, 0.2f);
    const auto undoKey = juce::KeyPress('z', juce::ModifierKeys::commandModifier, 0);
    auto* slider = findVisible<juce::Slider>(*editor, [](const juce::Slider& item) { return item.isVisible(); });
    REQUIRE(slider != nullptr);
    REQUIRE_FALSE(slider->keyPressed(undoKey));
    REQUIRE(rawValue(processor, openclapper::params::kToneLevel) == Catch::Approx(0.2f));
    REQUIRE(editor->keyPressed(undoKey));
    REQUIRE(rawValue(processor, openclapper::params::kToneLevel) == Catch::Approx(levelBefore));
    REQUIRE(editor->keyPressed(juce::KeyPress('z', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier, 0)));
    REQUIRE(rawValue(processor, openclapper::params::kToneLevel) == Catch::Approx(0.2f));

    auto* undo = findVisible<juce::TextButton>(*editor, [](const juce::TextButton& button) {
        return button.getButtonText() == "Undo";
    });
    auto* redo = findVisible<juce::TextButton>(*editor, [](const juce::TextButton& button) {
        return button.getButtonText() == "Redo";
    });
    REQUIRE(undo != nullptr);
    REQUIRE(redo != nullptr);
    REQUIRE(undo->getWidth() >= 44);
    REQUIRE(undo->getHeight() >= 36);
    undo->onClick();
    REQUIRE(rawValue(processor, openclapper::params::kToneLevel) == Catch::Approx(levelBefore));
    redo->onClick();
    REQUIRE(rawValue(processor, openclapper::params::kToneLevel) == Catch::Approx(0.2f));
}

TEST_CASE("A and B slots store a full state and copy either way", "[plugin][suite]") {
    OpenClapperProcessor processor;
    gesture(processor, openclapper::params::kToneDecay, 400.0f);
    processor.setParamLocked(openclapper::params::kPreSpread, true);
    REQUIRE(processor.canUndo());
    const float slotA = rawValue(processor, openclapper::params::kToneDecay);

    processor.toggleAb();
    REQUIRE_FALSE(processor.isParamLocked(openclapper::params::kPreSpread));
    REQUIRE(rawValue(processor, openclapper::params::kToneDecay) != Catch::Approx(slotA));
    REQUIRE_FALSE(processor.canUndo());
    gesture(processor, openclapper::params::kToneDecay, 90.0f);
    const float slotB = rawValue(processor, openclapper::params::kToneDecay);

    processor.toggleAb();
    REQUIRE(processor.isAbSlotB() == false);
    REQUIRE(rawValue(processor, openclapper::params::kToneDecay) == Catch::Approx(slotA));
    REQUIRE(processor.isParamLocked(openclapper::params::kPreSpread));
    processor.toggleAb();
    REQUIRE(rawValue(processor, openclapper::params::kToneDecay) == Catch::Approx(slotB));

    processor.copyAbBToA();
    processor.toggleAb();
    REQUIRE(rawValue(processor, openclapper::params::kToneDecay) == Catch::Approx(slotB));
    gesture(processor, openclapper::params::kToneDecay, 700.0f);
    processor.copyAbAToB();
    processor.toggleAb();
    REQUIRE(rawValue(processor, openclapper::params::kToneDecay) == Catch::Approx(700.0f));

    juce::MemoryBlock state;
    processor.getStateInformation(state);
    OpenClapperProcessor restored;
    restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    REQUIRE(restored.isAbSlotB());
    REQUIRE(rawValue(restored, openclapper::params::kToneDecay) == Catch::Approx(700.0f));
    restored.toggleAb();
    REQUIRE(rawValue(restored, openclapper::params::kToneDecay) == Catch::Approx(700.0f));
}

TEST_CASE("User presets save, rename, delete, search and favourite without touching factory sounds", "[plugin][suite]") {
    const auto directory = tempDirectory("presets");
    const auto documents = suite::UserPresetStore::defaultDirectory("openClapper");
    REQUIRE(documents.getFileName() == "Presets");
    REQUIRE(documents.getParentDirectory().getFileName() == "openClapper");
    REQUIRE(documents.getParentDirectory().getParentDirectory().getFileName() == "Alan Marcero");

    OpenClapperProcessor processor;
    processor.userPresets().setDirectory(directory);
    processor.setCurrentProgram(2);
    const float savedDecay = rawValue(processor, openclapper::params::kToneDecay);
    REQUIRE(processor.saveUserPreset("Room Clap"));
    processor.setCurrentProgram(8);
    REQUIRE(rawValue(processor, openclapper::params::kToneDecay) != Catch::Approx(savedDecay));
    processor.loadUserPreset("Room Clap");
    REQUIRE(processor.userPresetName() == "Room Clap");
    REQUIRE(processor.getSelectedPresetIndex() == -1);
    REQUIRE(rawValue(processor, openclapper::params::kToneDecay) == Catch::Approx(savedDecay));
    processor.undo();
    REQUIRE(processor.getSelectedPresetIndex() == 8);
    processor.redo();
    REQUIRE(processor.userPresetName() == "Room Clap");

    REQUIRE(processor.renameUserPreset("Room Clap", "Big Room"));
    REQUIRE(processor.userPresetName() == "Big Room");
    REQUIRE(processor.userPresets().load("Big Room").has_value());
    REQUIRE_FALSE(processor.userPresets().remove("Uplifting / Downbeat"));
    REQUIRE(processor.deleteUserPreset("Big Room"));
    REQUIRE(processor.userPresets().userNames().empty());
    REQUIRE(processor.userPresetName().isEmpty());

    processor.userPresets().setFavorite("Uplifting / Downbeat", true, true);
    REQUIRE(processor.userPresets().isFavorite("Uplifting / Downbeat", true));
    REQUIRE(processor.userPresets().userNames().empty());

    suite::PresetBrowser browser(processor.userPresets(), { "Uplifting / Downbeat", "Rolling / Two Hands" });
    browser.setSize(420, 440);
    browser.setVisible(true);
    browser.onSave = [&](const juce::String& name) { return processor.saveUserPreset(name); };
    browser.nameEditor().setText("Night Hit", juce::dontSendNotification);
    auto* save = findVisible<juce::TextButton>(browser, [](const juce::TextButton& button) {
        return button.getButtonText() == "Save";
    });
    REQUIRE(save != nullptr);
    REQUIRE(save->getHeight() >= 36);
    save->onClick();
    REQUIRE(processor.userPresets().load("Night Hit").has_value());

    browser.setSearchText("rolling");
    REQUIRE(browser.visibleCount() == 1);
    REQUIRE(browser.rowName(0) == "Rolling / Two Hands");
    browser.setSearchText("night");
    REQUIRE(browser.visibleCount() == 1);
    REQUIRE(browser.rowName(0) == "Night Hit");
    auto* list = findVisible<juce::ListBox>(browser, [](const juce::ListBox&) { return true; });
    REQUIRE(list != nullptr);
    list->selectRow(0);
    auto* remove = findVisible<juce::TextButton>(browser, [](const juce::TextButton& button) {
        return button.getButtonText() == "Delete";
    });
    REQUIRE(remove->isEnabled());
    browser.setSearchText("");
    REQUIRE(browser.visibleCount() >= 3);
    list->selectRow(2);
    REQUIRE(remove->isEnabled());
    list->selectRow(0);
    REQUIRE_FALSE(remove->isEnabled());
    directory.deleteRecursively();
}

TEST_CASE("Clicking a browser row loads factory and user presets", "[plugin][suite][gui]") {
    const auto directory = tempDirectory("row-clicks");
    OpenClapperProcessor processor;
    processor.userPresets().setDirectory(directory);
    REQUIRE(processor.saveUserPreset("Night Hit"));
    processor.selectPreset(0);
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    editor->setVisible(true);
    auto* open = findVisible<juce::TextButton>(*editor, [](const juce::TextButton& button) {
        return button.getButtonText() == "Presets";
    });
    REQUIRE(open != nullptr);
    open->onClick();
    auto* browser = findVisible<suite::PresetBrowser>(*editor, [](const suite::PresetBrowser&) { return true; });
    auto* list = findVisible<juce::ListBox>(*editor, [](const juce::ListBox&) { return true; });
    REQUIRE(browser != nullptr);
    REQUIRE(list != nullptr);
    const auto rowArea = [&](int row) { return editor->getLocalArea(list, list->getRowPosition(row, true)); };

    clickAt(*editor, rowArea(3).getCentre());
    REQUIRE(processor.getSelectedPresetIndex() == 3);
    REQUIRE(browser->isVisible());

    const auto starBefore = list->createComponentSnapshot(list->getRowPosition(1, true));
    clickAt(*editor, rowArea(1).getTopLeft() + juce::Point<int>(16, 18));
    REQUIRE(processor.userPresets().isFavorite(browser->rowName(1), true));
    REQUIRE(processor.getSelectedPresetIndex() == 3);
    REQUIRE_FALSE(samePixels(starBefore, list->createComponentSnapshot(list->getRowPosition(1, true))));

    REQUIRE(list->keyPressed(juce::KeyPress(juce::KeyPress::downKey)));
    REQUIRE(processor.getSelectedPresetIndex() == 3);
    REQUIRE(list->keyPressed(juce::KeyPress(juce::KeyPress::returnKey)));
    REQUIRE(processor.getSelectedPresetIndex() == 4);
    REQUIRE_FALSE(browser->isVisible());

    open->onClick();
    browser->setSearchText("night");
    clickAt(*editor, rowArea(0).getCentre());
    REQUIRE(processor.userPresetName() == "Night Hit");
    REQUIRE(processor.getSelectedPresetIndex() == -1);
    auto* remove = findVisible<juce::TextButton>(*browser, [](const juce::TextButton& button) {
        return button.getButtonText() == "Delete";
    });
    REQUIRE(remove != nullptr);
    REQUIRE(remove->isEnabled());
    directory.deleteRecursively();
}

TEST_CASE("Dragging a hit writes a WAV of the current sound", "[plugin][suite]") {
    const auto directory = tempDirectory("wav");
    OpenClapperProcessor processor;
    processor.setCurrentProgram(0);
    const auto first = processor.renderHitWav(directory.getChildFile("first.wav"));
    processor.setCurrentProgram(20);
    const auto second = processor.renderHitWav(directory.getChildFile("second.wav"));
    REQUIRE(first.existsAsFile());
    REQUIRE(second.existsAsFile());
    juce::MemoryBlock header;
    REQUIRE(first.loadFileAsData(header));
    REQUIRE(header.getSize() > 12);
    REQUIRE(std::memcmp(header.getData(), "RIFF", 4) == 0);

    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    auto read = [&](const juce::File& file) {
        std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(file));
        REQUIRE(reader != nullptr);
        REQUIRE(reader->numChannels == 2);
        juce::AudioBuffer<float> buffer(2, static_cast<int>(reader->lengthInSamples));
        reader->read(&buffer, 0, buffer.getNumSamples(), 0, true, true);
        return buffer.getMagnitude(0, buffer.getNumSamples());
    };
    const float peak = read(first);
    REQUIRE(peak > 0.01f);
    REQUIRE(std::abs(peak - read(second)) > 0.0001f);

    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    auto* drag = dynamic_cast<suite::WavDragButton*>(findVisible<juce::TextButton>(*editor, [](const juce::TextButton& button) {
        return button.getButtonText() == "Drag WAV";
    }));
    REQUIRE(drag != nullptr);
    REQUIRE(drag->getWidth() >= 44);
    REQUIRE(drag->getHeight() >= 36);
    REQUIRE(drag->exportFile().existsAsFile());
    directory.deleteRecursively();
}

TEST_CASE("Editor scales uniformly from 75% to 150% and remembers its size", "[plugin][suite][gui]") {
    OpenClapperProcessor processor;
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    REQUIRE(editor->getWidth() == 1080);
    REQUIRE(editor->getHeight() == 868);
    auto* constrainer = editor->getConstrainer();
    REQUIRE(constrainer != nullptr);
    REQUIRE(constrainer->getFixedAspectRatio() == Catch::Approx(1080.0 / 868.0));
    REQUIRE(constrainer->getMinimumWidth() == 810);
    REQUIRE(constrainer->getMinimumHeight() == 651);
    REQUIRE(constrainer->getMaximumWidth() == 1620);
    REQUIRE(constrainer->getMaximumHeight() == 1302);
    REQUIRE(editor->isResizable());

    auto* audition = findVisible<juce::TextButton>(*editor, [](const juce::TextButton& button) {
        return button.getButtonText() == "Audition A4";
    });
    REQUIRE(audition != nullptr);
    editor->setBoundsConstrained({ 0, 0, 2000, 1302 });
    REQUIRE(editor->getWidth() == 1620);
    REQUIRE(editor->getHeight() == 1302);
    const auto scaled = editor->getLocalArea(audition, audition->getLocalBounds());
    REQUIRE(scaled.getWidth() == audition->getWidth() * 3 / 2);
    REQUIRE(scaled.getHeight() == audition->getHeight() * 3 / 2);
    editor->setBoundsConstrained({ 0, 0, 400, 300 });
    REQUIRE(editor->getWidth() == 810);
    REQUIRE(editor->getHeight() == 651);
    editor->setBoundsConstrained({ 0, 0, 1296, 1042 });
    REQUIRE(processor.editorWidth() == 1296);
    REQUIRE(processor.editorHeight() == 1042);

    juce::MemoryBlock state;
    processor.getStateInformation(state);
    OpenClapperProcessor restored;
    restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    std::unique_ptr<juce::AudioProcessorEditor> reopened(restored.createEditor());
    REQUIRE(reopened->getWidth() == 1296);
    REQUIRE(reopened->getHeight() == 1042);
}

TEST_CASE("A free-aspect size from an older session opens at the scale that fits it", "[plugin][suite][gui]") {
    OpenClapperProcessor processor;
    processor.setEditorSize(1500, 980);
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    REQUIRE(editor->getHeight() == 980);
    REQUIRE(editor->getWidth() == juce::roundToInt(980.0 * 1080.0 / 868.0));
}

TEST_CASE("Older sessions keep the preset parameter when no host program was stored", "[plugin][suite]") {
    OpenClapperProcessor processor;
    processor.setCurrentProgram(43);
    processor.selectPreset(55);
    auto legacy = processor.parameters.copyState();
    legacy.setProperty("clap_bank", 3, nullptr);
    legacy.setProperty("selected_preset", 55, nullptr);
    legacy.removeProperty("host_program", nullptr);
    juce::MemoryBlock data;
    juce::AudioProcessor::copyXmlToBinary(*legacy.createXml(), data);

    OpenClapperProcessor restored;
    restored.setCurrentProgram(2);
    restored.setStateInformation(data.getData(), static_cast<int>(data.getSize()));
    REQUIRE(restored.getSelectedPresetIndex() == 55);
    REQUIRE(restored.getCurrentProgram() == 43);
    REQUIRE(restored.parameters.getParameter(openclapper::params::kPreset)->getValue() == Catch::Approx(1.0f));
    REQUIRE(restored.parameters.getRawParameterValue("amp_enabled")->load() == Catch::Approx(0.0f));
}

TEST_CASE("The preset name gains a star on the first edit and loses it on undo, load and save", "[plugin][suite]") {
    const auto directory = tempDirectory("edited");
    OpenClapperProcessor processor;
    processor.userPresets().setDirectory(directory);
    processor.selectPreset(4);
    CHECK(processor.presetDisplayName() == "Uplifting / Wide Ribbon");
    gesture(processor, openclapper::params::kToneDecay, 400.0f);
    CHECK(processor.presetDisplayName() == "Uplifting / Wide Ribbon *");
    processor.undo();
    CHECK(processor.presetDisplayName() == "Uplifting / Wide Ribbon");
    processor.redo();
    CHECK(processor.presetDisplayName() == "Uplifting / Wide Ribbon *");
    processor.selectPreset(4);
    CHECK(processor.presetDisplayName() == "Uplifting / Wide Ribbon");

    gesture(processor, openclapper::params::kToneDecay, 500.0f);
    REQUIRE(processor.saveUserPreset("Night Hit"));
    CHECK(processor.presetDisplayName() == "Night Hit");
    gesture(processor, openclapper::params::kNoiseDecay, 900.0f);
    CHECK(processor.presetDisplayName() == "Night Hit *");
    processor.loadUserPreset("Night Hit");
    CHECK(processor.presetDisplayName() == "Night Hit");
    gesture(processor, openclapper::params::kNoiseDecay, 700.0f);

    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    auto* header = findVisible<juce::ComboBox>(*editor, [](const juce::ComboBox& box) { return box.getName() == "Factory preset"; });
    REQUIRE(header != nullptr);
    CHECK(header->getText() == "Night Hit *");
    editor.reset();

    juce::MemoryBlock state;
    processor.getStateInformation(state);
    OpenClapperProcessor restored;
    restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    CHECK(restored.presetDisplayName() == "Night Hit *");
    restored.toggleAb();
    CHECK(restored.presetDisplayName() == "Uplifting / Downbeat");
    restored.toggleAb();
    CHECK(restored.presetDisplayName() == "Night Hit *");
    directory.deleteRecursively();
}

TEST_CASE("Each A/B slot keeps its own undo history, and copying into a slot clears it", "[plugin][suite]") {
    using openclapper::params::kToneDecay;
    OpenClapperProcessor processor;
    const float original = rawValue(processor, kToneDecay);
    gesture(processor, kToneDecay, 400.0f);
    gesture(processor, kToneDecay, 500.0f);
    processor.toggleAb();
    CHECK_FALSE(processor.canUndo());
    gesture(processor, kToneDecay, 90.0f);

    processor.toggleAb();
    REQUIRE(processor.canUndo());
    processor.undo();
    CHECK(rawValue(processor, kToneDecay) == Catch::Approx(400.0f));
    processor.toggleAb();
    CHECK(rawValue(processor, kToneDecay) == Catch::Approx(90.0f));
    REQUIRE(processor.canUndo());
    processor.undo();
    CHECK(rawValue(processor, kToneDecay) == Catch::Approx(original));
    CHECK_FALSE(processor.canUndo());

    processor.toggleAb();
    REQUIRE(processor.canUndo());
    processor.copyAbAToB();
    CHECK(processor.canUndo());
    processor.toggleAb();
    CHECK_FALSE(processor.canUndo());
    CHECK_FALSE(processor.canRedo());
}

TEST_CASE("Save asks before replacing and Delete asks before removing a user preset", "[plugin][suite]") {
    const auto directory = tempDirectory("confirm");
    OpenClapperProcessor processor;
    processor.userPresets().setDirectory(directory);
    suite::PresetBrowser browser(processor.userPresets(), { "Uplifting / Downbeat" });
    browser.setSize(420, 440);
    browser.setVisible(true);
    browser.onSave = [&](const juce::String& name) { return processor.saveUserPreset(name); };
    browser.onDelete = [&](const juce::String& name) { return processor.deleteUserPreset(name); };
    const auto button = [&](const char* text) {
        return findVisible<juce::TextButton>(browser, [text](const juce::TextButton& item) { return item.getButtonText() == text; });
    };
    const auto savedToneDecay = [&] {
        const auto tree = processor.userPresets().load("Night Hit");
        REQUIRE(tree.has_value());
        return static_cast<float>(tree->getChildWithProperty("id", openclapper::params::kToneDecay).getProperty("value"));
    };
    auto* save = button("Save");
    auto* remove = button("Delete");
    REQUIRE(save != nullptr);
    REQUIRE(remove != nullptr);

    gesture(processor, openclapper::params::kToneDecay, 300.0f);
    browser.nameEditor().setText("Night Hit", juce::dontSendNotification);
    save->onClick();
    CHECK(savedToneDecay() == Catch::Approx(300.0f));
    gesture(processor, openclapper::params::kToneDecay, 600.0f);
    save->onClick();
    CHECK(save->getButtonText() == "Replace?");
    CHECK(savedToneDecay() == Catch::Approx(300.0f));
    save->onClick();
    CHECK(save->getButtonText() == "Save");
    CHECK(savedToneDecay() == Catch::Approx(600.0f));

    browser.setSearchText("night");
    auto* list = findVisible<juce::ListBox>(browser, [](const juce::ListBox&) { return true; });
    REQUIRE(list != nullptr);
    list->selectRow(0);
    remove->onClick();
    CHECK(remove->getButtonText() == "Delete?");
    list->deselectAllRows();
    list->selectRow(0);
    CHECK(remove->getButtonText() == "Delete");
    remove->onClick();
    CHECK(processor.userPresets().load("Night Hit").has_value());
    remove->onClick();
    CHECK_FALSE(processor.userPresets().load("Night Hit").has_value());
    CHECK(remove->getButtonText() == "Delete");
    directory.deleteRecursively();
}
