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

TEST_CASE("Lock Pitch preserves note, octave, and fine pitch across preset changes", "[plugin][pitch_lock]") {
    OpenClapperProcessor processor;
    processor.prepareToPlay(48000.0, 512);

    auto* lockParam = processor.parameters.getParameter(openclapper::params::kLockPitch);
    REQUIRE(lockParam != nullptr);

    auto* octaveParam = processor.parameters.getParameter(openclapper::params::kToneOctave);
    auto* freqParam = processor.parameters.getParameter(openclapper::params::kToneFreq);
    auto* noteParam = processor.parameters.getParameter(openclapper::params::kToneNote);
    auto* fineParam = processor.parameters.getParameter(openclapper::params::kToneFine);
    REQUIRE(octaveParam != nullptr);
    REQUIRE(freqParam != nullptr);
    REQUIRE(noteParam != nullptr);
    REQUIRE(fineParam != nullptr);

    // Set custom high octave 7, frequency, note 4 (E), and fine cents 15
    octaveParam->setValueNotifyingHost(octaveParam->convertTo0to1(7.0f));
    freqParam->setValueNotifyingHost(freqParam->convertTo0to1(2800.0f));
    noteParam->setValueNotifyingHost(noteParam->convertTo0to1(4.0f));
    fineParam->setValueNotifyingHost(fineParam->convertTo0to1(15.0f));

    // Engage pitch lock
    lockParam->setValueNotifyingHost(1.0f);

    // Switch presets: octave, frequency, note, and fine cents must be preserved!
    processor.setCurrentProgram(1);
    REQUIRE(std::abs(processor.parameters.getRawParameterValue(openclapper::params::kToneOctave)->load() - 7.0f) < 0.05f);
    REQUIRE(std::abs(processor.parameters.getRawParameterValue(openclapper::params::kToneFreq)->load() - 2800.0f) < 2.0f);
    REQUIRE(std::abs(processor.parameters.getRawParameterValue(openclapper::params::kToneNote)->load() - 4.0f) < 0.05f);
    REQUIRE(std::abs(processor.parameters.getRawParameterValue(openclapper::params::kToneFine)->load() - 15.0f) < 0.5f);

    processor.setCurrentProgram(3); // Preset 4 has octave 2 default
    REQUIRE(std::abs(processor.parameters.getRawParameterValue(openclapper::params::kToneOctave)->load() - 7.0f) < 0.05f);
    REQUIRE(std::abs(processor.parameters.getRawParameterValue(openclapper::params::kToneFreq)->load() - 2800.0f) < 2.0f);
    REQUIRE(std::abs(processor.parameters.getRawParameterValue(openclapper::params::kToneNote)->load() - 4.0f) < 0.05f);
    REQUIRE(std::abs(processor.parameters.getRawParameterValue(openclapper::params::kToneFine)->load() - 15.0f) < 0.5f);

    // Disengage pitch lock: presets load their own pitch
    lockParam->setValueNotifyingHost(0.0f);
    processor.setCurrentProgram(3);
    REQUIRE(std::abs(processor.parameters.getRawParameterValue(openclapper::params::kToneOctave)->load() - 2.0f) < 0.05f);
}

TEST_CASE("Tone Octave supports high octaves up to 8", "[plugin][octave]") {
    OpenClapperProcessor processor;
    auto* octaveParam = processor.parameters.getParameter(openclapper::params::kToneOctave);
    REQUIRE(octaveParam != nullptr);

    octaveParam->setValueNotifyingHost(octaveParam->convertTo0to1(8.0f));
    REQUIRE(std::abs(processor.parameters.getRawParameterValue(openclapper::params::kToneOctave)->load() - 8.0f) < 0.05f);

    octaveParam->setValueNotifyingHost(octaveParam->convertTo0to1(6.0f));
    REQUIRE(std::abs(processor.parameters.getRawParameterValue(openclapper::params::kToneOctave)->load() - 6.0f) < 0.05f);
}
