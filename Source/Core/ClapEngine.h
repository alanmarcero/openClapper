#pragma once

#include "ClapSynth.h"
#include "Effects.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace openclapper::core {

struct Transport {
    double bpm = 138.0;
    std::optional<double> ppqPosition;
    double beatsPerBar = 4.0;
    bool isPlaying = true;
};

struct MidiTrigger {
    int sampleOffset = 0;
    int noteNumber = 69; // A4
    float velocity = 1.0f;
    bool audition = false;
    bool noteOff = false;
    int channel = 1;
};

struct MidiCcEvent {
    int sampleOffset = 0;
    int controller = 0;
    int value = 0;
};

struct ClapEffectSettings {
    DistortionType distType = DistortionType::Tape;
    float distDrive = 0.25f;       // 0..1
    float distTone = 0.0f;         // -1..+1
    float distMix = 0.35f;         // 0..1

    CompressorSettings compressor{};
    LimiterSettings limiter{};

    SlapbackDivision slapbackDivision = SlapbackDivision::Off;
    float slapbackMix = 0.3f;
    float slapbackFeedback = 0.0f;
    float doublerAmount = 0.0f;

    float reverbSize = 0.65f;
    float reverbDecayMs = 450.0f;
    float reverbDamp = 0.4f;
    float reverbGateMs = 160.0f;
    float reverbMix = 0.35f;

    ReverseSyncMode reverseMode = ReverseSyncMode::Off;
    float reverseMix = 0.5f;       // 0..1

    ThreeBandEqSettings eq{};
    float stereoWidth = 1.0f;      // 0..2
    float outputGainDb = 0.0f;     // -36..+12 dB
};

inline constexpr std::array<double, 5> kRollDivisionBeats{0.0, 0.5, 0.25, 0.125, 1.0 / 6.0};

struct EngineSettings {
    ClapSynthSettings synth{};
    ClapEffectSettings effects{};
    bool bypass = false;
    int keyLow = 0, keyHigh = 127;
    int velocityLow = 1, velocityHigh = 127;
    int chokeGroup = 0;
    int rollDivision = 0;
    int rollHits = 4;
};

class ClapEngine {
public:
    static constexpr int kLatencySamples = 0;

    void prepare(double sampleRate);
    void reset() noexcept;

    void process(
        const EngineSettings& settings,
        const Transport& transport,
        std::span<const MidiTrigger> midiTriggers,
        float* const* channels,
        int numChannels,
        int numSamples,
        std::span<const MidiCcEvent> controlChanges = {},
        std::span<float* const> componentChannels = {}) noexcept;

    void handleMidi(const EngineSettings& settings, const Transport& transport, const MidiTrigger& trigger) noexcept;

    [[nodiscard]] float currentPeak() const noexcept { return currentPeak_; }
    [[nodiscard]] float currentBeatPhase() const noexcept { return currentBeatPhase_; }
    [[nodiscard]] float currentClapProgress() const noexcept { return currentClapProgress_; }
    [[nodiscard]] uint32_t triggerSerial() const noexcept { return triggerSerial_; }
    [[nodiscard]] bool isReversing() const noexcept { return isReversing_; }

private:
    void triggerClap(
        const EngineSettings& settings,
        double samplePpq,
        double bpm,
        double beatsPerBar,
        float velocity,
        int midiNote,
        bool reverseCue) noexcept;

    [[nodiscard]] std::size_t allocateVoice() const noexcept;

    struct Voice {
        ClapSynth synth;
        uint64_t triggerOrder = 0;
        int note = 69, channel = 1, chokeGroup = 0;
        int auditionSamples = 0;
    };

    struct Roll {
        MidiTrigger trigger;
        double nextPpq = 0.0;
        double divisionBeats = 0.25;
        int remaining = 0;
        int chokeGroup = 0;
    };
    std::array<Roll, 8> rolls_{};
    ClapSynth reverseSynth_;

    double sampleRate_ = 48000.0;
    double fallbackPpq_ = 0.0;
    bool hasTransportPosition_ = false;
    bool pendingTrigger_ = false;
    std::array<Voice, 8> voices_{};
    uint64_t voiceOrderCounter_ = 0;
    std::size_t lastTriggeredVoice_ = 0;
    SlapbackDelay slapback_;
    GatedStereoReverb reverb_;
    DistortionShaper distL_;
    DistortionShaper distR_;
    ThreeBandEq eq_;
    StereoCompressor compressor_;
    BrickwallLimiter limiter_;

    // Reverse swell buffer
    std::vector<float> reverseBufferL_;
    std::vector<float> reverseBufferR_;
    std::vector<float> doublerBufferL_;
    std::vector<float> doublerBufferR_;
    size_t doublerWriteIndex_ = 0;
    float doublerLpL_ = 0.0f;
    float doublerLpR_ = 0.0f;
    int reversePlayHead_ = -1;
    int reverseDuration_ = 0;
    bool isReversing_ = false;

    // Performance CCs
    float ccVolume_ = 1.0f;
    float ccBrightness_ = 1.0f;

    // Metrics for GUI animation
    float currentPeak_ = 0.0f;
    float currentBeatPhase_ = 0.0f;
    float currentClapProgress_ = 1.0f; // 0 = clap triggered ("Clap on"), 1 = clap finished ("Clap off")
    uint32_t triggerSerial_ = 0;
};

} // namespace openclapper::core
