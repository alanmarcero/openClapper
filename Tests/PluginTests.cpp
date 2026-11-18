#include "Parameters.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Presets.h"
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include <ranges>
#include <utility>
#include <vector>

namespace {
template <typename Widget, typename Predicate>
Widget* findVisible(juce::Component& root, Predicate matches) {
    for (auto* child : root.getChildren()) {
        if (!child->isVisible()) continue;
        if (auto* widget = dynamic_cast<Widget*>(child); widget && matches(*widget)) return widget;
        if (auto* result = findVisible<Widget>(*child, matches)) return result;
    }
    return nullptr;
}

void click(juce::Component& editor, const juce::String& name) {
    auto* button = findVisible<juce::TextButton>(editor, [&](const auto& b) { return b.getButtonText() == name; });
    INFO(name);
    REQUIRE(button != nullptr);
    REQUIRE(button->getHeight() >= 36);
    button->onClick();
}

void checkTargets(juce::Component& root, juce::Component& parent) {
    for (auto* child : parent.getChildren()) {
        if (!child->isVisible()) continue;
        if (dynamic_cast<juce::TextButton*>(child) || dynamic_cast<juce::ComboBox*>(child)) {
            CAPTURE(child->getName());
            REQUIRE(child->getWidth() >= 44);
            REQUIRE(child->getHeight() >= 36);
            const auto bounds = root.getLocalArea(child, child->getLocalBounds());
            REQUIRE(root.getLocalBounds().contains(bounds));
            auto* target = root.getComponentAt(bounds.getCentre());
            REQUIRE((target == child || child->isParentOf(target)));
        }
        checkTargets(root, *child);
    }
}

void snapshot(juce::Component& editor, const char* filename) {
    checkTargets(editor, editor);
    const auto image = editor.createComponentSnapshot(editor.getLocalBounds());
    REQUIRE(image.isValid());
    const auto directory = juce::SystemStats::getEnvironmentVariable("OPENCLAPPER_SCREENSHOTS", {});
    if (directory.isEmpty()) return;
    auto file = juce::File(directory).getChildFile(filename);
    REQUIRE(file.deleteFile());
    juce::FileOutputStream stream(file);
    REQUIRE(stream.openedOk());
    REQUIRE(juce::PNGImageFormat().writeImageToStream(image, stream));
}
}

TEST_CASE("OpenClapperProcessor instantiates with valid parameters", "[plugin]") {
    OpenClapperProcessor processor;
    REQUIRE(processor.getName() == "openClapper");
    REQUIRE(processor.acceptsMidi());
    REQUIRE_FALSE(processor.producesMidi());

    REQUIRE(processor.parameters.getParameter(openclapper::params::kPreBursts) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kFlamSpread) != nullptr);
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
                            openclapper::params::kClickDecay, openclapper::params::kSlapbackFeedback,
                            openclapper::params::kClapDoubler, openclapper::params::kPitchHumanize }) {
        auto* parameter = processor.parameters.getParameter(id);
        REQUIRE(parameter != nullptr);
        CHECK(parameter->isAutomatable());
    }
}

TEST_CASE("Slapback feedback, stereo doubler, and pitch humanize shape output", "[plugin][dsp]") {
    openclapper::core::SlapbackDelay delayNoFb, delayFb;
    delayNoFb.prepare(48000.0);
    delayFb.prepare(48000.0);
    float l1 = 1.0f, r1 = 1.0f, l2 = 1.0f, r2 = 1.0f;
    (void)delayNoFb.process(l1, r1, openclapper::core::SlapbackDivision::Sixteenth, 0.5f, 120.0, true, 0.0f);
    (void)delayFb.process(l2, r2, openclapper::core::SlapbackDivision::Sixteenth, 0.5f, 120.0, true, 0.75f);
    float lateEnergyNoFb = 0.0f, lateEnergyFb = 0.0f;
    for (int i = 0; i < 16000; ++i) {
        l1 = 0.0f; r1 = 0.0f; l2 = 0.0f; r2 = 0.0f;
        (void)delayNoFb.process(l1, r1, openclapper::core::SlapbackDivision::Sixteenth, 0.5f, 120.0, false, 0.0f);
        (void)delayFb.process(l2, r2, openclapper::core::SlapbackDivision::Sixteenth, 0.5f, 120.0, false, 0.75f);
        if (i > 8000) {
            lateEnergyNoFb += std::abs(l1);
            lateEnergyFb += std::abs(l2);
        }
    }
    REQUIRE(lateEnergyFb > lateEnergyNoFb + 0.05f);

    OpenClapperProcessor procDry, procDoubler;
    procDry.prepareToPlay(48000.0, 4096);
    procDoubler.prepareToPlay(48000.0, 4096);
    auto* doublerParam = procDoubler.parameters.getParameter(openclapper::params::kClapDoubler);
    REQUIRE(doublerParam != nullptr);
    doublerParam->setValueNotifyingHost(1.0f);
    juce::AudioBuffer<float> bufDry(2, 4096), bufDoubler(2, 4096);
    juce::MidiBuffer midiDry, midiDoubler;
    midiDry.addEvent(juce::MidiMessage::noteOn(1, 69, 1.0f), 0);
    midiDoubler.addEvent(juce::MidiMessage::noteOn(1, 69, 1.0f), 0);
    procDry.processBlock(bufDry, midiDry);
    procDoubler.processBlock(bufDoubler, midiDoubler);
    float diff = 0.0f;
    for (int i = 0; i < 4096; ++i) {
        diff += std::abs(bufDry.getSample(0, i) - bufDoubler.getSample(0, i));
    }
    REQUIRE(diff > 0.1f);
}

TEST_CASE("OpenClapperProcessor retains 44 host programs and offers 56 editor presets", "[plugin]") {
    OpenClapperProcessor processor;
    REQUIRE(processor.getNumPrograms() == 44);
    REQUIRE(openclapper::presets::factoryPresets().size() == 56);

    for (int i = 0; i < 44; ++i) {
        processor.setCurrentProgram(i);
        REQUIRE(processor.getCurrentProgram() == i);
        REQUIRE(processor.getProgramName(i).isNotEmpty());
    }
}

TEST_CASE("All factory presets render and use tuned noise", "[plugin][presets]") {
    OpenClapperProcessor processor;
    processor.prepareToPlay(48000.0, 8192);
    juce::AudioBuffer<float> buffer(2, 8192);

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

}

TEST_CASE("Factory claps leave headroom and finish inside a trance backbeat", "[plugin][bank]") {
    const auto& bank = openclapper::presets::factoryPresets();
    for (size_t i = 0; i < bank.size(); ++i) {
        INFO(bank[i].name);
        OpenClapperProcessor processor;
        processor.selectPreset(static_cast<int>(i));
        REQUIRE(processor.parameters.getRawParameterValue("amp_enabled")->load() < 0.5f);
        processor.prepareToPlay(48000, 24000);
        juce::AudioBuffer<float> audio(2, 24000);
        juce::MidiBuffer midi;
        midi.addEvent(juce::MidiMessage::noteOn(1, 69, 1.0f), 0);
        processor.processBlock(audio, midi);
        const auto peak = audio.getMagnitude(0, audio.getNumSamples());
        double total = 0, cumulative = 0;
        std::array<double, 500> byMs{};
        for (int s = 0; s < audio.getNumSamples(); ++s) {
            const double left = audio.getSample(0, s), right = audio.getSample(1, s);
            byMs[static_cast<size_t>(s / 48)] += left * left + right * right;
        }
        for (auto energy : byMs) total += energy;
        int endMs = 0;
        while (endMs < 500 && cumulative < total * 0.99)
            cumulative += byMs[static_cast<size_t>(endMs++)];
        CAPTURE(peak, endMs);
        REQUIRE(std::isfinite(total));
        REQUIRE(peak > 0.14f);
        REQUIRE(peak < 1.0f);
        REQUIRE(endMs > 5);
        REQUIRE(endMs < 180);
        REQUIRE(audio.getMagnitude(14400, 9600) < peak * 0.004f);
    }
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
    proc1.setCurrentProgram(5);
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

TEST_CASE("Native editor browses, auditions and edits clap layers and held notes", "[plugin][gui]") {
    OpenClapperProcessor processor;
    processor.prepareToPlay(48000, 512);
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    REQUIRE(editor != nullptr);
    editor->setVisible(true);
    auto* browser = findVisible<juce::ComboBox>(*editor, [](const auto& b) { return b.getName() == "Factory preset"; });
    REQUIRE(browser != nullptr);
    REQUIRE(browser->getNumItems() == 56);
    CHECK(browser->getRootMenu()->getNumItems() == 6);
    snapshot(*editor, "openClapper-screenshot.png");
    click(*editor, "Previous");
    REQUIRE(processor.getSelectedPresetIndex() == 55);
    REQUIRE(processor.getCurrentProgram() == 0);
    click(*editor, "Next");
    REQUIRE(processor.getSelectedPresetIndex() == 0);
    browser->setSelectedId(13, juce::sendNotificationSync);
    REQUIRE(processor.getSelectedPresetIndex() == 12);
    click(*editor, "Audition A4");
    juce::AudioBuffer<float> audio(2, 512);
    juce::MidiBuffer midi;
    processor.processBlock(audio, midi);
    REQUIRE(audio.getMagnitude(0, 512) > 0.01f);

    const auto edit = [&](const char* id, float value) {
        auto* slider = findVisible<juce::Slider>(*editor, [id](const auto& control) {
            return control.getParentComponent()->getComponentID() == id;
        });
        INFO(id);
        REQUIRE(slider != nullptr);
        slider->setValue(value, juce::sendNotificationSync);
        REQUIRE(processor.parameters.getRawParameterValue(id)->load() == Catch::Approx(value));
    };
    click(*editor, "Layers");
    edit("pre_decay", 6);
    edit("noise_hold", 4);
    edit("tone_decay", 60);
    edit("click_decay", 2);
    snapshot(*editor, "openClapper-screenshot-advanced.png");
    click(*editor, "Effects");
    edit("reverb_gate", 100);
    snapshot(*editor, "openClapper-effects.png");
    click(*editor, "MIDI & envelope");
    click(*editor, "Hold notes");
    REQUIRE(processor.parameters.getRawParameterValue("amp_enabled")->load() == Catch::Approx(1.0f));
    edit("amp_attack", 5);
    edit("amp_decay", 160);
    edit("amp_sustain", 0.75f);
    edit("amp_release", 420);
    snapshot(*editor, "openClapper-performance.png");
    click(*editor, "One-shot");
    REQUIRE(processor.parameters.getRawParameterValue("amp_enabled")->load() == Catch::Approx(0.0f));
    click(*editor, "Clap");
}

TEST_CASE("SVG hands follow host tempo and seeks, rest on stop, and pulse without a host", "[plugin][gui][clock]") {
    struct Playhead : juce::AudioPlayHead {
        PositionInfo info;
        juce::Optional<PositionInfo> getPosition() const override { return info; }
    } host;
    OpenClapperProcessor processor;
    processor.setRateAndBufferSizeDetails(48000, 480);
    processor.prepareToPlay(48000, 480);
    processor.setPlayHead(&host);
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    const auto render = [&] {
        juce::MidiBuffer midi;
        juce::AudioBuffer<float> audio(2, 480);
        processor.processBlock(audio, midi);
        return editor->createComponentSnapshot({editor->getWidth() - 97, 90, 82, 54});
    };
    const auto difference = [](const juce::Image& a, const juce::Image& b) {
        int count = 0;
        for (int y = 0; y < a.getHeight(); ++y)
            for (int x = 0; x < a.getWidth(); ++x)
                if (a.getPixelAt(x,y) != b.getPixelAt(x,y)) ++count;
        return count;
    };
    host.info.setIsPlaying(true);
    host.info.setBpm(120);
    host.info.setPpqPosition(8);
    const auto downbeat = render();
    REQUIRE(processor.displayBeatPhase.load() == Catch::Approx(0.02f));
    host.info.setBpm(90);
    host.info.setPpqPosition(-1.5);
    const auto offbeat = render();
    REQUIRE(processor.displayBeatPhase.load() == Catch::Approx(0.515f));
    REQUIRE(processor.displayBpm.load() == Catch::Approx(90.0f));
    REQUIRE(difference(downbeat, offbeat) > 80);
    host.info.setIsPlaying(false);
    const auto stopped = render();
    host.info.setPpqPosition(16);
    REQUIRE(difference(stopped, render()) == 0);
    REQUIRE(processor.displayBeatPhase.load() < 0.0f);
    processor.setPlayHead(nullptr);
    render();
    REQUIRE(processor.displayBpm.load() == Catch::Approx(138.0f));
    const auto internal = processor.displayBeatPhase.load();
    render();
    REQUIRE(processor.displayBeatPhase.load() == Catch::Approx(internal + 0.023f));
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

TEST_CASE("Old bank states recall their held sound as a custom patch", "[plugin][performance]") {
    OpenClapperProcessor processor;
    performanceValue(processor, "amp_enabled", 1);
    performanceValue(processor, "amp_release", 20000);
    auto legacy = processor.parameters.copyState();
    legacy.removeProperty("clap_bank", nullptr);
    legacy.setProperty("selected_preset", 44, nullptr);
    juce::MemoryBlock data;
    juce::AudioProcessor::copyXmlToBinary(*legacy.createXml(), data);
    processor.selectPreset(1);
    processor.setStateInformation(data.getData(), static_cast<int>(data.getSize()));
    REQUIRE(processor.getSelectedPresetIndex() == -1);
    REQUIRE(processor.parameters.getRawParameterValue("amp_enabled")->load() == Catch::Approx(1.0f));
    REQUIRE(processor.parameters.getRawParameterValue("amp_release")->load() == Catch::Approx(20000.0f));
    processor.getStateInformation(data);
    OpenClapperProcessor restored;
    restored.setStateInformation(data.getData(), static_cast<int>(data.getSize()));
    REQUIRE(restored.getSelectedPresetIndex() == -1);
    REQUIRE(restored.parameters.getRawParameterValue("amp_enabled")->load() == Catch::Approx(1.0f));
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
    processor.selectPreset(55);
    REQUIRE(processor.getCurrentProgram() == 43);
    REQUIRE(processor.getSelectedPresetIndex() == 55);
    REQUIRE(processor.getProgramName(55) == "Classic / Last Accent");
    REQUIRE(selector->getValue() == Catch::Approx(1.0f));
    REQUIRE(processor.parameters.getRawParameterValue(openclapper::params::kPreset)->load() == Catch::Approx(43.0f));
    juce::MemoryBlock impactState;
    processor.getStateInformation(impactState);
    restored.setStateInformation(impactState.getData(), static_cast<int>(impactState.getSize()));
    REQUIRE(restored.getCurrentProgram() == 43);
    REQUIRE(restored.getSelectedPresetIndex() == 55);
    REQUIRE(restored.parameters.getRawParameterValue("amp_enabled")->load() == Catch::Approx(0.0f));
}

TEST_CASE("OpenClapper exposes 3-band EQ, main-page Output, and section/per-part locks", "[plugin][ui][locks][eq]") {
    openclapper::core::ThreeBandEq eqBoost, eqCut;
    eqBoost.prepare(48000.0);
    eqCut.prepare(48000.0);
    const openclapper::core::ThreeBandEqSettings boost{ 80.0f, 12.0f, 1000.0f, 0.0f, 7500.0f, 0.0f };
    const openclapper::core::ThreeBandEqSettings cut{ 80.0f, -12.0f, 1000.0f, 0.0f, 7500.0f, 0.0f };
    double rmsBoost = 0.0;
    double rmsCut = 0.0;
    std::ranges::for_each(std::views::iota(0, 4800), [&](int i) {
        const float x = 0.25f * std::sin(2.0f * 3.14159265f * 30.0f * static_cast<float>(i) / 48000.0f);
        float bL = x;
        float bR = x;
        eqBoost.process(bL, bR, boost);
        float cL = x;
        float cR = x;
        eqCut.process(cL, cR, cut);
        if (i >= 2400) {
            rmsBoost += static_cast<double>(bL) * bL;
            rmsCut += static_cast<double>(cL) * cL;
        }
    });
    REQUIRE(10.0 * std::log10(rmsBoost / rmsCut) > 20.0);

    OpenClapperProcessor processor;
    std::ranges::for_each(std::array<const char*, 6>{
        "eq_low_freq", "eq_low_db", "eq_mid_freq", "eq_mid_db", "eq_high_freq", "eq_high_db"},
        [&](const char* id) { REQUIRE(processor.parameters.getParameter(id) != nullptr); });

    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    REQUIRE(editor != nullptr);
    editor->setVisible(true);
    checkTargets(*editor, *editor);

    const auto findById = [](auto&& self, juce::Component& parent, const juce::String& id) -> juce::Component* {
        if (parent.getComponentID() == id) return &parent;
        for (auto* child : parent.getChildren()) {
            if (auto* found = self(self, *child, id)) return found;
        }
        return nullptr;
    };

    std::ranges::for_each(std::array<const char*, 5>{
        "section_lock_Hand spacing", "section_lock_Tuned body", "section_lock_Noise tail",
        "section_lock_Finish", "section_lock_Output"},
        [&](const char* id) {
            auto* button = dynamic_cast<juce::TextButton*>(findById(findById, *editor, id));
            REQUIRE(button != nullptr);
            CHECK(button->isVisible());
        });

    std::ranges::for_each(std::array<const char*, 16>{
        "lock_pre_bursts", "lock_pre_spread", "lock_tone_level", "lock_tone_q",
        "lock_noise_type", "lock_noise_decay", "lock_click_level", "lock_reverb_mix",
        "lock_eq_low_db", "lock_eq_low_freq", "lock_eq_mid_db", "lock_eq_high_db",
        "lock_stereo_width", "lock_output_gain", "lock_limiter_ceiling", "lock_limiter_enabled"},
        [&](const char* id) {
            auto* badge = findById(findById, *editor, id);
            REQUIRE(badge != nullptr);
            CHECK(badge->isVisible());
        });

    auto* finishLockBtn = dynamic_cast<juce::TextButton*>(findById(findById, *editor, "section_lock_Finish"));
    REQUIRE(finishLockBtn != nullptr);
    finishLockBtn->onClick();
    REQUIRE(processor.isSectionLocked({ openclapper::params::kClickLevel, openclapper::params::kReverbMix, openclapper::params::kClapDoubler }));

    performanceValue(processor, openclapper::params::kClickLevel, 0.91f);
    performanceValue(processor, openclapper::params::kPreSpread, 33.0f);
    performanceValue(processor, openclapper::params::kToneQ, 9.5f);
    performanceValue(processor, openclapper::params::kNoiseDecay, 410.0f);
    processor.setParamLocked(openclapper::params::kPreSpread, true);
    processor.setParamLocked(openclapper::params::kToneQ, true);
    processor.selectPreset(3);
    CHECK(processor.parameters.getRawParameterValue(openclapper::params::kClickLevel)->load() == Catch::Approx(0.91f));
    CHECK(processor.parameters.getRawParameterValue(openclapper::params::kPreSpread)->load() == Catch::Approx(33.0f));
    CHECK(processor.parameters.getRawParameterValue(openclapper::params::kToneQ)->load() == Catch::Approx(9.5f));
    CHECK(processor.parameters.getRawParameterValue(openclapper::params::kNoiseDecay)->load() != Catch::Approx(410.0f));

    juce::MemoryBlock state;
    processor.getStateInformation(state);
    OpenClapperProcessor restored;
    restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    CHECK(restored.isParamLocked(openclapper::params::kPreSpread));
    CHECK(restored.isParamLocked(openclapper::params::kToneQ));
    CHECK(restored.isParamLocked(openclapper::params::kClickLevel));
}
