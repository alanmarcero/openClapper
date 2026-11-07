#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <string_view>
#include <vector>

namespace openclapper::core {

inline constexpr int kNumNotes = 12;

inline constexpr std::array<std::string_view, kNumNotes> kNoteNames = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

enum class NoiseType {
    White = 0,
    Pink = 1,
    Velvet = 2,
    LoFi = 3
};

struct ClapSynthSettings {
    // Pitch & Tuning
    int note = 9;                 // 0..11 (A), default A
    int octave = 3;               // 1..5, default 3
    float fineCents = 0.0f;       // -50..+50 cents

    // Pre-Clap Flam (Micro-bursts)
    int preClapCount = 3;         // 1..5 micro-bursts before main slap
    float preClapSpreadMs = 12.0f;// 2..45 ms spacing between bursts
    float preClapDecayMs = 8.0f;  // 2..35 ms decay of each burst
    float preClapJitter = 0.15f;  // 0..1 timing and pitch humanization
    float preClapLevel = 0.85f;   // 0..1 volume of pre-claps

    // Resonant Body / Tone
    float toneFreqHz = 1100.0f;   // 200..4000 Hz center frequency
    float toneQ = 3.2f;           // 0.5..12.0 resonance
    float toneDecayMs = 120.0f;   // 15..600 ms tone decay
    float tonePitchEnv = 7.0f;    // -24..+24 semitones pitch drop
    float tonePitchDecayMs = 18.0f;// 3..80 ms pitch decay
    float toneLevel = 0.75f;      // 0..1 level of tone core

    // Noise & Sizzle
    NoiseType noiseType = NoiseType::White;
    float noiseAttackMs = 0.0f;   // 0..20 ms
    float noiseHoldMs = 2.0f;     // 0..30 ms
    float noiseDecayMs = 180.0f;  // 20..1000 ms
    float noiseCurve = -0.3f;     // -1..+1 shape curve (-1 exp, 0 lin, +1 log)
    float noiseHPF = 350.0f;      // 20..2500 Hz
    float noiseLPF = 9500.0f;     // 800..20000 Hz
    float noiseStereoWidth = 1.0f;// 0 (mono) .. 2 (ultra-wide)
    float noiseLevel = 0.85f;     // 0..1

    // Click / Transient
    float clickLevel = 0.4f;      // 0..1
    float clickFreqHz = 4500.0f;  // 1000..12000 Hz

    // Gated Reverb Tail
    float reverbSize = 0.65f;     // 0.1..1.0
    float reverbDecayMs = 450.0f; // 50..2500 ms
    float reverbDamp = 0.4f;      // 0..1 high damping
    float reverbGateMs = 160.0f;  // 0 (off) .. 600 ms
    float reverbMix = 0.35f;      // 0..1

    bool operator==(const ClapSynthSettings&) const noexcept = default;
};

// Computes base tuned frequency from note index, octave, and fine cents.
[[nodiscard]] inline float noteToHz(int noteIndex, int octave, float fineCents = 0.0f) noexcept {
    const int note = (noteIndex % kNumNotes + kNumNotes) % kNumNotes;
    const int oct = std::clamp(octave, 1, 5);
    // MIDI note: Octave 4 C is 60, Octave 4 A is 69 (440 Hz)
    const int midiNote = 12 * (oct + 1) + note;
    const float totalNotes = static_cast<float>(midiNote - 69) + (fineCents / 100.0f);
    return 440.0f * std::pow(2.0f, totalNotes / 12.0f);
}

// Biquad filter state
struct BiquadState {
    float x1 = 0.0f;
    float x2 = 0.0f;
    float y1 = 0.0f;
    float y2 = 0.0f;

    void reset() noexcept {
        x1 = x2 = y1 = y2 = 0.0f;
    }
};

// Clean stereo clap frame
struct StereoFrame {
    float left = 0.0f;
    float right = 0.0f;
};

// Algorithmic stereo diffuser / reverb with gate
class GatedStereoReverb {
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    [[nodiscard]] StereoFrame process(float inLeft, float inRight, float size, float decayMs, float damp, float gateMs, double sampleRate) noexcept;

private:
    std::vector<float> delayL1_;
    std::vector<float> delayL2_;
    std::vector<float> delayR1_;
    std::vector<float> delayR2_;
    int writeL1_ = 0, writeL2_ = 0;
    int writeR1_ = 0, writeR2_ = 0;
    float dampStateL_ = 0.0f;
    float dampStateR_ = 0.0f;
    int sampleCounter_ = 0;
    float gateEnvelope_ = 1.0f;
};

// Pure C++20 Clap Synthesizer
class ClapSynth {
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    void trigger(const ClapSynthSettings& settings, float velocity = 1.0f, int semitoneOffset = 0) noexcept;
    [[nodiscard]] StereoFrame processSample(const ClapSynthSettings& settings) noexcept;
    [[nodiscard]] bool isPlaying() const noexcept { return isPlaying_; }
    [[nodiscard]] float getEnvelopeProgress() const noexcept;

    // Direct render block for offline preview, tests, or WAV lock-in
    void renderBlock(
        std::vector<float>& leftOut,
        std::vector<float>& rightOut,
        const ClapSynthSettings& settings,
        float velocity = 1.0f,
        int semitoneOffset = 0) noexcept;

private:
    [[nodiscard]] float generateNoiseSample(NoiseType type) noexcept;

    double sampleRate_ = 48000.0;
    bool isPlaying_ = false;
    float velocity_ = 1.0f;
    int semitoneOffset_ = 0;

    int totalSamplesPlayed_ = 0;
    int totalDurationSamples_ = 0;

    // Micro-burst pre-clap timing
    int preClapCount_ = 3;
    std::array<int, 5> burstStartSamples_{};
    std::array<float, 5> burstAmplitudes_{};
    int mainSlapSample_ = 0;

    // Filter states
    BiquadState toneFilterL_{};
    BiquadState toneFilterR_{};
    BiquadState noiseHPF_L_{};
    BiquadState noiseHPF_R_{};
    BiquadState noiseLPF_L_{};
    BiquadState noiseLPF_R_{};
    BiquadState clickBPF_{};

    // PRNG seed for deterministic and jittered noise
    uint32_t prngState_ = 0x12345678;
    float pinkB0_ = 0.0f, pinkB1_ = 0.0f, pinkB2_ = 0.0f;
    float lofiHeld_ = 0.0f;
    int lofiCounter_ = 0;

    // Delay buffer for stereo width
    std::vector<float> widthDelayBuffer_;
    int widthDelayWritePos_ = 0;

    // Gated Reverb
    GatedStereoReverb reverb_;
};

} // namespace openclapper::core
