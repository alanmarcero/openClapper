#pragma once

#include "ClapEngine.h"
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>

class OpenClapperProcessor : public juce::AudioProcessor {
public:
    OpenClapperProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;
    double getTailLengthSeconds() const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "openClapper"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram_; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void triggerAudition() noexcept { auditionPending_.store(true); }

    juce::AudioProcessorValueTreeState parameters;

    std::atomic<float> displayPeak{ 0.0f };
    std::atomic<float> displayBeatPhase{ 0.0f };
    std::atomic<float> displayBpm{ 138.0f };
    std::atomic<float> displayBeatsPerBar{ 4.0f };
    std::atomic<float> displayClapProgress{ 1.0f };
    std::atomic<float> displayVelocity{ 1.0f };
    std::atomic<uint32_t> displayTriggerCount{ 0 };

private:
    openclapper::core::ClapEngine engine_;
    std::atomic<bool> auditionPending_{ false };
    int currentProgram_ = 0;
};
