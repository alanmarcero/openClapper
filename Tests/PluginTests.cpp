#include "Parameters.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Presets.h"
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include <utility>
#include <vector>

namespace {
juce::File findRepositoryRoot() {
#if defined(OPENCLAPPER_SOURCE_DIR)
    const juce::File configuredRoot(OPENCLAPPER_SOURCE_DIR);
    if (configuredRoot.getChildFile("assets").isDirectory())
        return configuredRoot;
#endif
    auto directory = juce::File::getCurrentWorkingDirectory();
    while (directory.exists()) {
        if (directory.getChildFile("assets").isDirectory())
            return directory;
        const auto parent = directory.getParentDirectory();
        if (parent == directory)
            break;
        directory = parent;
    }
    return juce::File::getCurrentWorkingDirectory();
}

void checkButtonHitAreas(juce::Component& component) {
    for (auto* child : component.getChildren()) {
        if (auto* button = dynamic_cast<juce::TextButton*>(child)) {
            INFO(button->getButtonText());
            CHECK(button->getHeight() >= 36);
            CHECK(button->getLookAndFeel().getTextButtonFont(*button, button->getHeight()).getHeight() >= 14.0f);
        }
        checkButtonHitAreas(*child);
    }
}
}

TEST_CASE("OpenClapperProcessor instantiates with valid parameters", "[plugin]") {
    OpenClapperProcessor processor;
    REQUIRE(processor.getName() == "openClapper");
    REQUIRE(processor.acceptsMidi());
    REQUIRE_FALSE(processor.producesMidi());

    REQUIRE(processor.parameters.getParameter(openclapper::params::kPreBursts) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kFlamSpread) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kToneMode) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kClickType) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kNoiseType) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kDistDrive) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kCompThresh) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kLimiterCeiling) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kReverbMix) != nullptr);
}

TEST_CASE("OpenClapper exposes automatable body sweep, body filters, and transient decay", "[plugin][parameters]") {
    OpenClapperProcessor processor;
    for (const auto* id : { openclapper::params::kTonePitchEnv, openclapper::params::kTonePitchDecay,
                            openclapper::params::kToneHPF, openclapper::params::kToneLPF,
                            openclapper::params::kClickDecay }) {
        auto* parameter = processor.parameters.getParameter(id);
        REQUIRE(parameter != nullptr);
        CHECK(parameter->isAutomatable());
    }
}

TEST_CASE("OpenClapperProcessor retains 44 host programs and offers 45 editor presets", "[plugin]") {
    OpenClapperProcessor processor;
    REQUIRE(processor.getNumPrograms() == 44);
    REQUIRE(openclapper::presets::factoryPresets().size() == 45);

    for (int i = 0; i < 44; ++i) {
        processor.setCurrentProgram(i);
        REQUIRE(processor.getCurrentProgram() == i);
        REQUIRE(processor.getProgramName(i).isNotEmpty());
    }
}

TEST_CASE("All factory presets render and Bandpass patches use tuned noise", "[plugin][presets]") {
    OpenClapperProcessor processor;
    processor.prepareToPlay(48000.0, 8192);
    juce::AudioBuffer<float> buffer(2, 8192);
    bool foundBandpass = false;

    for (int index = 0; index < processor.getNumPrograms(); ++index) {
        processor.setCurrentProgram(index);
        buffer.clear();
        juce::MidiBuffer midi;
        midi.addEvent(juce::MidiMessage::noteOn(1, 69, static_cast<juce::uint8>(100)), 0);
        processor.processBlock(buffer, midi);

        float peak = 0.0f;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                const float value = buffer.getSample(channel, sample);
                REQUIRE(std::isfinite(value));
                peak = std::max(peak, std::abs(value));
            }
        }
        REQUIRE(peak > 0.00001f);

        auto settings = openclapper::params::readSettings(processor.parameters).synth;
        if (settings.toneMode != openclapper::core::ToneMode::Bandpass)
            continue;

        foundBandpass = true;
        settings.preClapCount = 0;
        settings.noiseLevel = 0.0f;
        settings.clickLevel = 0.0f;
        auto alternate = settings;
        alternate.noiseType = settings.noiseType == openclapper::core::NoiseType::White
            ? openclapper::core::NoiseType::Pink
            : openclapper::core::NoiseType::White;

        openclapper::core::ClapSynth originalSynth;
        openclapper::core::ClapSynth alternateSynth;
        originalSynth.prepare(48000.0);
        alternateSynth.prepare(48000.0);
        std::vector<float> originalLeft, originalRight, alternateLeft, alternateRight;
        originalSynth.renderBlock(originalLeft, originalRight, settings, 1.0f, 69);
        alternateSynth.renderBlock(alternateLeft, alternateRight, alternate, 1.0f, 69);
        float difference = 0.0f;
        for (size_t sample = 0; sample < std::min(originalLeft.size(), alternateLeft.size()); ++sample)
            difference += std::abs(originalLeft[sample] - alternateLeft[sample]);
        REQUIRE(difference > 0.01f);
    }

    REQUIRE(foundBandpass);
}

TEST_CASE("OpenClapperProcessor renders audio upon MIDI note-on", "[plugin]") {
    OpenClapperProcessor processor;
    processor.prepareToPlay(48000.0, 512);

    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 69, (juce::uint8)100), 0);

    processor.processBlock(buffer, midi);

    float maxSample = 0.0f;
    for (int ch = 0; ch < 2; ++ch) {
        for (int i = 0; i < 512; ++i) {
            maxSample = std::max(maxSample, std::abs(buffer.getSample(ch, i)));
        }
    }

    REQUIRE(maxSample > 0.0f);
    REQUIRE(processor.displayPeak.load() > 0.0f);
    REQUIRE(processor.displayTriggerCount.load() >= 1);
}

TEST_CASE("OpenClapperProcessor saves and restores state information", "[plugin]") {
    OpenClapperProcessor proc1;
    proc1.setCurrentProgram(5); // Dutch Hardstyle Smacker
    const std::array<std::pair<const char*, float>, 5> featureValues {{
        { openclapper::params::kTonePitchEnv, -8.5f },
        { openclapper::params::kTonePitchDecay, 110.0f },
        { openclapper::params::kToneHPF, 650.0f },
        { openclapper::params::kToneLPF, 7200.0f },
        { openclapper::params::kClickDecay, 3.2f }
    }};
    for (const auto& [id, value] : featureValues) {
        auto* parameter = proc1.parameters.getParameter(id);
        REQUIRE(parameter != nullptr);
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    }
    for (const auto& parameter : openclapper::params::kPerformanceParameters) {
        auto* value = proc1.parameters.getParameter(parameter.id);
        value->setValueNotifyingHost(0.7f);
    }

    juce::MemoryBlock block;
    proc1.getStateInformation(block);
    REQUIRE(block.getSize() > 0);

    OpenClapperProcessor proc2;
    proc2.setStateInformation(block.getData(), static_cast<int>(block.getSize()));

    const float p1 = proc1.parameters.getRawParameterValue(openclapper::params::kDistDrive)->load();
    const float p2 = proc2.parameters.getRawParameterValue(openclapper::params::kDistDrive)->load();
    REQUIRE(std::abs(p1 - p2) < 0.001f);
    for (const auto& parameter : openclapper::params::kPerformanceParameters)
        REQUIRE(std::abs(proc1.parameters.getRawParameterValue(parameter.id)->load()
            - proc2.parameters.getRawParameterValue(parameter.id)->load()) < 0.001f);
    for (const auto& [id, value] : featureValues)
        REQUIRE(std::abs(proc2.parameters.getRawParameterValue(id)->load() - value) < 0.11f);
}

TEST_CASE("OpenClapperEditor instantiates with correct dimensions", "[plugin][gui]") {
    OpenClapperProcessor processor;
    processor.prepareToPlay(48000.0, 512);

    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    REQUIRE(editor != nullptr);
    CHECK(editor->getWidth() == 1200);
    CHECK(editor->getHeight() == 840);
    checkButtonHitAreas(*editor);

    juce::TextButton* performanceButton = nullptr;
    for (auto* child : editor->getChildren()) {
        auto* button = dynamic_cast<juce::TextButton*>(child);
        if (button != nullptr && button->getButtonText() == "PERFORMANCE") performanceButton = button;
    }
    REQUIRE(performanceButton != nullptr);
    juce::ComboBox* editorPresets = nullptr;
    for (auto* child : editor->getChildren())
        if (auto* box = dynamic_cast<juce::ComboBox*>(child)) editorPresets = box;
    REQUIRE(editorPresets != nullptr);
    REQUIRE(editorPresets->getNumItems() == 45);
    editorPresets->setSelectedId(45, juce::sendNotificationSync);
    REQUIRE(processor.getSelectedPresetIndex() == 44);
    REQUIRE(processor.getCurrentProgram() == 0);
    REQUIRE(processor.parameters.getRawParameterValue("amp_release")->load() == Catch::Approx(20000.0f));
    juce::Image snapshot = editor->createComponentSnapshot(editor->getLocalBounds());
    REQUIRE(snapshot.isValid());
    CHECK(snapshot.getWidth() == 1200);
    CHECK(snapshot.getHeight() == 840);
    juce::TextButton* viewButton = nullptr;
    for (auto* child : editor->getChildren()) {
        auto* button = dynamic_cast<juce::TextButton*>(child);
        if (button != nullptr && button->getButtonText() == "SIMPLE VIEW") viewButton = button;
    }
    REQUIRE(viewButton != nullptr);
    viewButton->onClick();
    const auto advancedSnapshot = editor->createComponentSnapshot(editor->getLocalBounds());
    REQUIRE(advancedSnapshot.isValid());
    const auto advancedFile = findRepositoryRoot().getChildFile("assets/openClapper-screenshot-advanced.png");
    advancedFile.deleteFile();
    juce::FileOutputStream advancedImage(advancedFile);
    REQUIRE(advancedImage.openedOk());
    juce::PNGImageFormat advancedPng;
    REQUIRE(advancedPng.writeImageToStream(advancedSnapshot, advancedImage));
    viewButton->onClick();
    performanceButton->setToggleState(true, juce::dontSendNotification);
    performanceButton->onClick();
    auto performanceSnapshot = editor->createComponentSnapshot(editor->getLocalBounds());
    auto performanceFile = findRepositoryRoot().getChildFile("assets/openClapper-performance.png");
    performanceFile.deleteFile();
    juce::FileOutputStream performanceImage(performanceFile);
    REQUIRE(performanceImage.openedOk());
    juce::PNGImageFormat performancePng;
    REQUIRE(performancePng.writeImageToStream(performanceSnapshot, performanceImage));

    juce::File repoScreenshot = findRepositoryRoot().getChildFile("assets/openClapper-screenshot.png");
    if (repoScreenshot.getParentDirectory().exists()) {
        repoScreenshot.deleteFile();
        juce::FileOutputStream stream(repoScreenshot);
        if (stream.openedOk()) {
            juce::PNGImageFormat png;
            png.writeImageToStream(snapshot, stream);
        }
    }

    juce::File tempFile = juce::File::createTempFile(".png");
    {
        juce::FileOutputStream stream(tempFile);
        REQUIRE(stream.openedOk());
        juce::PNGImageFormat png;
        REQUIRE(png.writeImageToStream(snapshot, stream));
    }
    REQUIRE(tempFile.existsAsFile());
    REQUIRE(tempFile.getSize() > 0);
    tempFile.deleteFile();
}

TEST_CASE("Performance controls expose all approved clap features", "[plugin][performance]") {
    OpenClapperProcessor processor;
    for (const auto* id : {"body_velocity", "noise_velocity", "click_velocity", "body_mute", "body_solo",
                           "noise_mute", "noise_solo", "click_mute", "click_solo", "pre_mute", "pre_solo",
                           "key_low", "key_high", "velocity_low", "velocity_high", "choke_group",
                           "roll_division", "roll_hits", "amp_enabled", "amp_attack", "amp_decay", "amp_sustain", "amp_release"}) {
        INFO(id);
        REQUIRE(processor.parameters.getParameter(id) != nullptr);
    }
    REQUIRE(processor.getBusCount(false) == 5);
}

namespace {
void performanceValue(OpenClapperProcessor& processor, const char* id, float value) {
    auto* parameter = processor.parameters.getParameter(id);
    REQUIRE(parameter != nullptr);
    parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}
float performanceRender(OpenClapperProcessor& processor, int note = 69, float velocity = 1.0f) {
    processor.prepareToPlay(48000, 4096);
    juce::AudioBuffer<float> audio(2, 4096);
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, note, velocity), 0);
    processor.processBlock(audio, midi);
    return audio.getMagnitude(0, audio.getNumSamples());
}
}

TEST_CASE("MIDI zones reject notes and velocities outside inclusive bounds", "[plugin][performance]") {
    OpenClapperProcessor processor;
    performanceValue(processor, "key_low", 60);
    performanceValue(processor, "key_high", 72);
    performanceValue(processor, "velocity_low", 64);
    REQUIRE(performanceRender(processor, 59) <= 0.0000001f);
    REQUIRE(performanceRender(processor, 69, 0.25f) <= 0.0000001f);
    REQUIRE(performanceRender(processor, 60) > 0.0f);
    REQUIRE(performanceRender(processor, 72) > 0.0f);
}

TEST_CASE("Muting all components silences synthesis and solo selects a component", "[plugin][performance]") {
    OpenClapperProcessor processor;
    for (const auto* id : {"body_mute", "noise_mute", "click_mute", "pre_mute"})
        performanceValue(processor, id, 1);
    REQUIRE(performanceRender(processor) <= 0.0000001f);
    performanceValue(processor, "body_mute", 0);
    performanceValue(processor, "body_solo", 1);
    REQUIRE(performanceRender(processor) > 0.0f);
}

TEST_CASE("Rolls count finite host tempo retriggers", "[plugin][performance]") {
    OpenClapperProcessor processor;
    performanceValue(processor, "roll_division", 3);
    performanceValue(processor, "roll_hits", 4);
    processor.prepareToPlay(48000, 48000);
    juce::AudioBuffer<float> audio(2, 48000);
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 69, 1.0f), 0);
    processor.processBlock(audio, midi);
    REQUIRE(processor.displayTriggerCount.load() == 4);
}

TEST_CASE("ADSR release accepts twenty seconds and survives note off", "[plugin][performance]") {
    OpenClapperProcessor processor;
    performanceValue(processor, "amp_enabled", 1);
    performanceValue(processor, "amp_release", 20000);
    REQUIRE(processor.parameters.getRawParameterValue("amp_release")->load() == Catch::Approx(20000.0f));
    REQUIRE(processor.getTailLengthSeconds() >= 20.0);
    processor.prepareToPlay(48000, 48000);
    juce::AudioBuffer<float> audio(2, 48000);
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 69, 1.0f), 0);
    midi.addEvent(juce::MidiMessage::noteOff(1, 69), 24000);
    processor.processBlock(audio, midi);
    REQUIRE(audio.getMagnitude(36000, 12000) > 0.001f);
}

TEST_CASE("Body velocity response can ignore velocity independently", "[plugin][performance]") {
    OpenClapperProcessor quiet, loud;
    for (auto* processor : {&quiet, &loud}) {
        performanceValue(*processor, "body_solo", 1);
        performanceValue(*processor, "body_velocity", 0);
    }
    REQUIRE(std::abs(performanceRender(quiet, 69, 0.25f) - performanceRender(loud)) < 0.00001f);
}

TEST_CASE("Component buses deliver independent body noise click and pre-claps", "[plugin][performance]") {
    OpenClapperProcessor processor;
    REQUIRE(processor.getBusCount(false) == 5);
    auto layout = processor.getBusesLayout();
    for (int bus = 1; bus < 5; ++bus)
        layout.outputBuses.set(bus, juce::AudioChannelSet::stereo());
    REQUIRE(processor.setBusesLayout(layout));
    processor.prepareToPlay(48000, 4096);
    juce::AudioBuffer<float> audio(10, 4096);
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 69, 1.0f), 0);
    processor.processBlock(audio, midi);
    for (int bus = 1; bus < 5; ++bus) {
        INFO(bus);
        REQUIRE(audio.getMagnitude(bus * 2, 0, 4096) > 0.00001f);
    }
}

TEST_CASE("Choke groups stop previous voices on a new accepted note", "[plugin][performance]") {
    OpenClapperProcessor processor;
    performanceValue(processor, "amp_enabled", 1);
    performanceValue(processor, "body_solo", 1);
    performanceValue(processor, "choke_group", 1);
    processor.prepareToPlay(48000, 1024);
    juce::AudioBuffer<float> audio(2, 1024);
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 60, 1.0f), 0);
    processor.processBlock(audio, midi);
    midi.clear();
    midi.addEvent(juce::MidiMessage::noteOn(1, 69, 1.0f), 0);
    midi.addEvent(juce::MidiMessage::noteOff(1, 69), 1);
    performanceValue(processor, "amp_release", 0);
    processor.processBlock(audio, midi);
    midi.clear();
    processor.processBlock(audio, midi);
    processor.processBlock(audio, midi);
    // Shared effects are disabled so a held old voice cannot hide in an effects tail.
    performanceValue(processor, "reverb_mix", 0);
    performanceValue(processor, "dist_mix", 0);
    performanceValue(processor, "comp_mix", 0);
    processor.processBlock(audio, midi);
    REQUIRE(audio.getMagnitude(0, 1024) <= 0.0000001f);
}

TEST_CASE("Impact Clap factory preset sustains a prominent twenty second noise release", "[plugin][performance]") {
    OpenClapperProcessor processor;
    int impact = -1;
    const auto& bank = openclapper::presets::factoryPresets();
    for (size_t index = 0; index < bank.size(); ++index)
        if (juce::String(bank[index].name).contains("Impact Clap")) impact = static_cast<int>(index);
    REQUIRE(impact >= 0);
    processor.selectPreset(impact);
    REQUIRE(processor.parameters.getRawParameterValue("amp_release")->load() == Catch::Approx(20000.0f));
    REQUIRE(processor.parameters.getRawParameterValue(openclapper::params::kNoiseLevel)->load() >= 0.9f);
    processor.prepareToPlay(48000, 48000);
    juce::AudioBuffer<float> audio(2, 48000);
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 69, 1.0f), 0);
    midi.addEvent(juce::MidiMessage::noteOff(1, 69), 12000);
    processor.processBlock(audio, midi);
    midi.clear();
    for (int second = 0; second < 10; ++second)
        processor.processBlock(audio, midi);
    REQUIRE(audio.getMagnitude(0, 48000) > 0.001f);
}

TEST_CASE("Loading an old state resets absent performance controls to compatibility defaults", "[plugin][performance]") {
    OpenClapperProcessor processor;
    auto oldState = processor.parameters.copyState();
    for (int child = oldState.getNumChildren(); --child >= 0;) {
        const auto id = oldState.getChild(child).getProperty("id").toString();
        if (id == "amp_enabled" || id == "body_mute" || id == "roll_division")
            oldState.removeChild(child, nullptr);
    }
    performanceValue(processor, "amp_enabled", 1);
    performanceValue(processor, "body_mute", 1);
    performanceValue(processor, "roll_division", 3);
    juce::MemoryBlock state;
    auto xml = oldState.createXml();
    juce::AudioProcessor::copyXmlToBinary(*xml, state);
    processor.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    REQUIRE(processor.parameters.getRawParameterValue("amp_enabled")->load() <= 0.0000001f);
    REQUIRE(processor.parameters.getRawParameterValue("body_mute")->load() <= 0.0000001f);
    REQUIRE(processor.parameters.getRawParameterValue("roll_division")->load() <= 0.0000001f);
}

TEST_CASE("Host tail includes the last hit of a finite fill", "[plugin][performance]") {
    OpenClapperProcessor processor;
    performanceValue(processor, "roll_division", 1);
    performanceValue(processor, "roll_hits", 32);
    REQUIRE(processor.getTailLengthSeconds() > 6.7);
}

TEST_CASE("Audition releases a full ADSR patch automatically", "[plugin][performance]") {
    OpenClapperProcessor processor;
    performanceValue(processor, "amp_enabled", 1);
    performanceValue(processor, "amp_release", 200);
    performanceValue(processor, openclapper::params::kReverbMix, 0);
    performanceValue(processor, openclapper::params::kDistMix, 0);
    performanceValue(processor, openclapper::params::kCompMix, 0);
    processor.prepareToPlay(48000, 48000);
    processor.triggerAudition();
    juce::AudioBuffer<float> audio(2, 48000);
    juce::MidiBuffer midi;
    processor.processBlock(audio, midi);
    REQUIRE(audio.getMagnitude(0, 12000) > 0.001f);
    REQUIRE(audio.getMagnitude(36000, 12000) < 0.0000001f);
}

TEST_CASE("Legacy preset automation preserves all original normalized program values", "[plugin][performance]") {
    OpenClapperProcessor processor, restored;
    auto* selector = processor.parameters.getParameter(openclapper::params::kPreset);
    for (int program = 0; program < 44; ++program) {
        INFO(program);
        const float legacyAutomation = static_cast<float>(program) / 43.0f;
        selector->setValueNotifyingHost(legacyAutomation);
        REQUIRE(processor.parameters.getRawParameterValue(openclapper::params::kPreset)->load()
            == Catch::Approx(static_cast<float>(program)));
        juce::MemoryBlock state;
        processor.getStateInformation(state);
        restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        REQUIRE(restored.parameters.getParameter(openclapper::params::kPreset)->getValue()
            == Catch::Approx(legacyAutomation));
    }
    processor.setCurrentProgram(43);
    processor.selectPreset(44);
    REQUIRE(processor.getCurrentProgram() == 43);
    REQUIRE(processor.getSelectedPresetIndex() == 44);
    REQUIRE(processor.getProgramName(44).contains("Impact Clap"));
    REQUIRE(selector->getValue() == Catch::Approx(1.0f));
    REQUIRE(processor.parameters.getRawParameterValue(openclapper::params::kPreset)->load() == Catch::Approx(43.0f));
    juce::MemoryBlock impactState;
    processor.getStateInformation(impactState);
    restored.setStateInformation(impactState.getData(), static_cast<int>(impactState.getSize()));
    REQUIRE(restored.getCurrentProgram() == 43);
    REQUIRE(restored.getSelectedPresetIndex() == 44);
    REQUIRE(restored.parameters.getRawParameterValue("amp_release")->load() == Catch::Approx(20000.0f));
}
