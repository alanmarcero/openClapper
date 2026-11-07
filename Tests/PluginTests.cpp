#include "Parameters.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Presets.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>

TEST_CASE("OpenClapperProcessor instantiates with valid parameters", "[plugin]") {
    OpenClapperProcessor processor;
    REQUIRE(processor.getName() == "openClapper");
    REQUIRE(processor.acceptsMidi());
    REQUIRE_FALSE(processor.producesMidi());

    REQUIRE(processor.parameters.getParameter(openclapper::params::kPreBursts) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kFlamSpread) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kToneMode) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kToneFreq) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kClickType) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kNoiseType) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kDistDrive) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kCompThresh) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kLimiterCeiling) != nullptr);
    REQUIRE(processor.parameters.getParameter(openclapper::params::kReverbMix) != nullptr);
}

TEST_CASE("OpenClapperProcessor provides exactly 32 factory presets", "[plugin]") {
    OpenClapperProcessor processor;
    REQUIRE(processor.getNumPrograms() == 32);

    for (int i = 0; i < 32; ++i) {
        processor.setCurrentProgram(i);
        REQUIRE(processor.getCurrentProgram() == i);
        REQUIRE(processor.getProgramName(i).isNotEmpty());
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
    proc1.setCurrentProgram(5); // Dutch Hardstyle Smacker

    juce::MemoryBlock block;
    proc1.getStateInformation(block);
    REQUIRE(block.getSize() > 0);

    OpenClapperProcessor proc2;
    proc2.setStateInformation(block.getData(), static_cast<int>(block.getSize()));

    const float p1 = proc1.parameters.getRawParameterValue(openclapper::params::kDistDrive)->load();
    const float p2 = proc2.parameters.getRawParameterValue(openclapper::params::kDistDrive)->load();
    REQUIRE(std::abs(p1 - p2) < 0.001f);
}

TEST_CASE("OpenClapperEditor instantiates with correct dimensions", "[plugin][gui]") {
    OpenClapperProcessor processor;
    processor.prepareToPlay(48000.0, 512);

    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    REQUIRE(editor != nullptr);
    REQUIRE(editor->getWidth() == 1100);
    REQUIRE(editor->getHeight() == 740);

    juce::Image snapshot = editor->createComponentSnapshot(editor->getLocalBounds());
    REQUIRE(snapshot.isValid());
    REQUIRE(snapshot.getWidth() == 1100);
    REQUIRE(snapshot.getHeight() == 740);

    juce::File outputFile("/Users/alan.marcero/Development/openClapper/assets/openClapper-screenshot.png");
    outputFile.deleteFile();
    juce::FileOutputStream stream(outputFile);
    juce::PNGImageFormat png;
    REQUIRE(png.writeImageToStream(snapshot, stream));
    REQUIRE(outputFile.existsAsFile());
}
