#include "Parameters.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Presets.h"
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <vector>

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

TEST_CASE("OpenClapperProcessor provides exactly 44 factory presets", "[plugin]") {
    OpenClapperProcessor processor;
    REQUIRE(processor.getNumPrograms() == 44);

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

    juce::File repoScreenshot = juce::File::getCurrentWorkingDirectory().getChildFile("assets/openClapper-screenshot.png");
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
