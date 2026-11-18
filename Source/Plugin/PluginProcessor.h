#pragma once

#include "ClapEngine.h"
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <set>

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
    int getCurrentProgram() override { return currentProgram_.load(); }
    void setCurrentProgram(int index) override;
    void selectPreset(int index);
    [[nodiscard]] int getSelectedPresetIndex() const noexcept { return selectedPreset_.load(); }
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void triggerAudition() noexcept { auditionPending_.store(true); }

    [[nodiscard]] bool isParamLocked(const juce::String& id) const;
    void setParamLocked(const juce::String& id, bool locked);
    [[nodiscard]] bool isSectionLocked(const juce::StringArray& ids) const;
    void toggleSectionLock(const juce::StringArray& ids);
    [[nodiscard]] juce::StringArray lockedParameterIds() const;

    juce::AudioProcessorValueTreeState parameters;

    std::atomic<float> displayPeak{ 0.0f };
    std::atomic<float> displayBeatPhase{ 0.0f };
    std::atomic<float> displayBpm{ 138.0f };
    std::atomic<float> displayBeatsPerBar{ 4.0f };
    std::atomic<float> displayClapProgress{ 1.0f };
    std::atomic<float> displayVelocity{ 1.0f };
    std::atomic<uint32_t> displayTriggerCount{ 0 };

private:
    void syncLocksToState();
    void loadLocksFromState(const juce::ValueTree& state);
    openclapper::core::ClapEngine engine_;
    std::atomic<bool> auditionPending_{ false };
    std::atomic<int> currentProgram_{0}, selectedPreset_{0};
    mutable juce::CriticalSection lockMutex_;
    std::set<juce::String> lockedParams_;
};
