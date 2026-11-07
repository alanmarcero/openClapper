#pragma once

#include "ClapSynth.h"
#include "Effects.h"
#include <atomic>
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

    ReverseSyncMode reverseMode = ReverseSyncMode::Off;
    float reverseMix = 0.5f;       // 0..1

    float stereoWidth = 1.0f;      // 0..2
    float outputGainDb = 0.0f;     // -36..+12 dB
};

struct EngineSettings {
    ClapSynthSettings synth{};
    ClapEffectSettings effects{};
    bool noteTracking = false;     // false = Fixed (audition root), true = Note tracking
    bool lockIn = false;           // Lock in rendered WAV buffer
    bool bypass = false;
};

// Internal 32-bit float stereo WAV cache for deterministic Lock-In mode
class ClapWavCache {
public:
    void clear() noexcept;
    void capture(const std::vector<float>& left, const std::vector<float>& right, double sampleRate);
    [[nodiscard]] bool isLocked() const noexcept { return isLocked_; }
    [[nodiscard]] double sampleRate() const noexcept { return sampleRate_; }
    [[nodiscard]] const std::vector<float>& left() const noexcept { return left_; }
    [[nodiscard]] const std::vector<float>& right() const noexcept { return right_; }

    [[nodiscard]] std::vector<uint8_t> toWavBytes() const;
    bool fromWavBytes(const std::vector<uint8_t>& wavBytes);

private:
    bool isLocked_ = false;
    double sampleRate_ = 48000.0;
    std::vector<float> left_;
    std::vector<float> right_;
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
        std::span<const MidiCcEvent> controlChanges = {}) noexcept;

    void lockCurrentClap(const EngineSettings& settings);
    void unlockClap() noexcept;
    bool restoreLockedWav(const std::vector<uint8_t>& wavBytes);

    [[nodiscard]] bool isClapLocked() const noexcept { return wavCache_.isLocked(); }
    [[nodiscard]] const ClapWavCache& wavCache() const noexcept { return wavCache_; }

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
        int semitoneOffset,
        bool reverseCue) noexcept;

    double sampleRate_ = 48000.0;
    double fallbackPpq_ = 0.0;
    bool previousLockState_ = false;

    ClapSynth synth_;
    DistortionShaper distL_;
    DistortionShaper distR_;
    StereoCompressor compressor_;
    BrickwallLimiter limiter_;

    ClapWavCache wavCache_;
    int wavPlayHead_ = -1;
    float wavVelocity_ = 1.0f;

    // Reverse swell buffer
    std::vector<float> reverseBufferL_;
    std::vector<float> reverseBufferR_;
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
