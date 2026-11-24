#pragma once

#include "ClapEngine.h"
#include "suite/AbSlots.h"
#include "suite/ParameterUndo.h"
#include "suite/UserPresets.h"
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <mutex>
#include <set>

class OpenClapperProcessor : public juce::AudioProcessor,
                            private juce::AudioProcessorParameter::Listener,
                            private juce::AsyncUpdater {
public:
    OpenClapperProcessor();
    ~OpenClapperProcessor() override;

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

    void undo() { undo_.undo(); }
    void redo() { undo_.redo(); }
    [[nodiscard]] bool canUndo() const { return undo_.canUndo(); }
    [[nodiscard]] bool canRedo() const { return undo_.canRedo(); }

    void toggleAb();
    void copyAbAToB();
    void copyAbBToA();
    [[nodiscard]] bool isAbSlotB() const noexcept { return ab_.editingB(); }

    [[nodiscard]] juce::String userPresetName() const;
    // The loaded user or factory preset name, or "Custom sound".
    [[nodiscard]] juce::String presetName() const;
    // presetName() with " *" after any parameter edit since the last load or save.
    [[nodiscard]] juce::String presetDisplayName() const { return presetName() + (presetEdited_.load() ? " *" : ""); }
    void loadUserPreset(const juce::String& name);
    bool saveUserPreset(const juce::String& name);
    bool renameUserPreset(const juce::String& from, const juce::String& to);
    bool deleteUserPreset(const juce::String& name);
    suite::UserPresetStore& userPresets() noexcept { return userPresets_; }

    void setEditorSize(int width, int height) noexcept;
    [[nodiscard]] int editorWidth() const noexcept { return editorWidth_.load(); }
    [[nodiscard]] int editorHeight() const noexcept { return editorHeight_.load(); }

    /** Offline render of the current hit, using the same A4 audition as the editor. */
    [[nodiscard]] juce::File renderHitWav(const juce::File& destination);
    void serviceLockIn();
    [[nodiscard]] bool isClapLockedIn() const noexcept;

    juce::AudioProcessorValueTreeState parameters;

    // The editor reads these two with exchange(). Each keeps its extreme since the last read.
    std::atomic<float> displayPeak{ 0.0f };
    std::atomic<float> displayLimiterGain{ 1.0f };
    std::atomic<float> displayBeatPhase{ 0.0f };
    std::atomic<float> displayBpm{ 138.0f };
    std::atomic<float> displayBeatsPerBar{ 4.0f };
    std::atomic<float> displayClapProgress{ 1.0f };
    std::atomic<float> displayVelocity{ 1.0f };
    std::atomic<uint32_t> displayTriggerCount{ 0 };
    std::atomic<uint32_t> displayDroppedCount{ 0 };

private:
    enum class ApplyKind { session, slot, undo };

    void syncLocksToState();
    void loadLocksFromState(const juce::ValueTree& state);
    [[nodiscard]] juce::ValueTree captureSoundState();
    void applySoundTree(const juce::ValueTree& incoming, ApplyKind kind);
    void applyParameterChildren(const juce::ValueTree& tree, const juce::StringArray& lockedIds);
    void setUserPresetName(juce::String name);
    void parameterValueChanged(int index, float newValue) override;
    void parameterGestureChanged(int, bool) override {}
    void handleAsyncUpdate() override;
    [[nodiscard]] juce::AudioBuffer<float> renderHitBuffer(double& sampleRate) const;
    [[nodiscard]] bool playFrozen(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi,
                                  const openclapper::core::Transport& transport);
    void storeFreeze(juce::AudioBuffer<float> buffer, double sampleRate);
    void clearFreeze();
    [[nodiscard]] bool writeWavFile(const juce::File& destination, const juce::AudioBuffer<float>& buffer, double sampleRate) const;

    struct Freeze {
        juce::AudioBuffer<float> audio;
        double sampleRate = 48000.0;
    };

    openclapper::core::ClapEngine engine_;
    std::atomic<bool> auditionPending_{ false };
    std::atomic<int> currentProgram_{0}, selectedPreset_{0};
    std::atomic<int> editorWidth_{0}, editorHeight_{0};
    mutable juce::CriticalSection lockMutex_;
    std::set<juce::String> lockedParams_;
    juce::String userPresetName_;
    // Host automation can call parameterValueChanged on the audio thread, so both are atomic.
    std::atomic<bool> presetEdited_{ false };
    std::atomic<int> loading_{ 0 };
    int presetParameterIndex_ = -1;
    int lockParameterIndex_ = -1;
    std::shared_ptr<const Freeze> freeze_;
    mutable std::mutex freezeMutex_;
    std::atomic<bool> lockRenderPending_{ false };
    std::atomic<bool> clearLockRequested_{ false };
    std::atomic<double> freezePlayhead_{ -1.0 };
    std::atomic<uint32_t> freezeTriggers_{ 0 };
    bool playingFreeze_ = false;
    suite::ParameterUndo undo_;
    suite::AbSlots ab_;
    suite::UserPresetStore userPresets_{ suite::UserPresetStore::defaultDirectory("openClapper") };
};
