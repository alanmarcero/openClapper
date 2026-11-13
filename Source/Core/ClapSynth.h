#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <vector>

namespace openclapper::core {

enum class NoiseType {
    White = 0,
    Pink = 1,
    Velvet = 2,
    LoFi = 3
};

enum class ToneMode {
    Bandpass = 0,
    SubThump = 1,
    SaturatedTri = 2,
    CyberSquare = 3
};

enum class ClickType {
    Needle = 0,
    Crack = 1,
    Thump = 2
};

struct ClapSynthSettings {
    // Pre-Clap Flam (Micro-bursts)
    int preClapCount = 3;         // 1..5 micro-bursts before main slap
    float preClapSpreadMs = 12.0f;// 2..45 ms spacing between bursts
    float preClapDecayMs = 8.0f;  // 2..35 ms decay of each burst
    float preClapJitter = 0.15f;  // 0..1 timing and pitch humanization
    float preClapFlamSpread = 0.45f; // 0..1 stereo micro-panning spread
    float preClapLevel = 0.85f;   // 0..1 volume of pre-claps

    // Resonant Body / Tone
    ToneMode toneMode = ToneMode::Bandpass;
    float toneQ = 3.2f;           // 0.5..12.0 resonance
    float toneDecayMs = 120.0f;   // 15..600 ms tone decay
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
    ClickType clickType = ClickType::Needle;
    float clickLevel = 0.4f;      // 0..1
    float clickFreqHz = 4500.0f;  // 1000..12000 Hz

    bool operator==(const ClapSynthSettings&) const noexcept = default;
};

[[nodiscard]] inline float midiNoteToHz(int midiNote) noexcept {
    const int note = std::clamp(midiNote, 0, 127);
    return 440.0f * std::pow(2.0f, static_cast<float>(note - 69) / 12.0f);
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

// Stereo allpass diffuser for smooth transient dispersion without metallic comb flutter
struct AllpassDiffuser {
    std::vector<float> buffer;
    size_t writeIndex = 0;
    float feedback = 0.6f;

    void prepare(size_t delaySamples, float g = 0.6f) {
        buffer.assign(std::max<size_t>(1, delaySamples), 0.0f);
        writeIndex = 0;
        feedback = g;
    }

    void reset() noexcept {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writeIndex = 0;
    }

    [[nodiscard]] float process(float in) noexcept {
        if (buffer.empty()) {
            return in;
        }
        const float delayed = buffer[writeIndex];
        const float out = -feedback * in + delayed;
        buffer[writeIndex] = in + feedback * delayed;
        writeIndex = (writeIndex + 1) % buffer.size();
        return out;
    }
};

// Algorithmic stereo diffuser / reverb with gate
class GatedStereoReverb {
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    void triggerGate() noexcept;
    [[nodiscard]] StereoFrame process(float inLeft, float inRight, float size, float decayMs, float damp, float gateMs, double sampleRate) noexcept;

private:
    [[nodiscard]] static float readHermite(const std::vector<float>& buffer, double delaySamples, int writePos) noexcept;

    AllpassDiffuser diffL1_;
    AllpassDiffuser diffL2_;
    AllpassDiffuser diffL3_;
    AllpassDiffuser diffR1_;
    AllpassDiffuser diffR2_;
    AllpassDiffuser diffR3_;
    std::vector<float> delayL1_;
    std::vector<float> delayL2_;
    std::vector<float> delayL3_;
    std::vector<float> delayR1_;
    std::vector<float> delayR2_;
    std::vector<float> delayR3_;
    int writeL1_ = 0, writeL2_ = 0, writeL3_ = 0;
    int writeR1_ = 0, writeR2_ = 0, writeR3_ = 0;
    float dampStateL1_ = 0.0f, dampStateL2_ = 0.0f;
    float dampStateR1_ = 0.0f, dampStateR2_ = 0.0f;
    float hpfStateL_ = 0.0f, hpfStateR_ = 0.0f;
    double lfoPhaseL_ = 0.0;
    double lfoPhaseR_ = 0.0;
    int sampleCounter_ = 0;
    float gateEnvelope_ = 1.0f;
};

// Procedural MIDI-pitched clap voice
class ClapSynth {
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    void trigger(const ClapSynthSettings& settings, int midiNote, float velocity = 1.0f) noexcept;
    [[nodiscard]] StereoFrame processSample(const ClapSynthSettings& settings) noexcept;
    [[nodiscard]] bool isPlaying() const noexcept { return isPlaying_; }
    [[nodiscard]] float getEnvelopeProgress() const noexcept;

    // Direct render block for offline preview and reverse swells.
    void renderBlock(
        std::vector<float>& leftOut,
        std::vector<float>& rightOut,
        const ClapSynthSettings& settings,
        float velocity = 1.0f,
        int midiNote = 69) noexcept;

private:
    [[nodiscard]] float generateNoiseSample(NoiseType type) noexcept;

    double sampleRate_ = 48000.0;
    bool isPlaying_ = false;
    float velocity_ = 1.0f;
    int midiNote_ = 69;
    uint32_t triggerCount_ = 0;

    int totalSamplesPlayed_ = 0;
    int totalDurationSamples_ = 0;
    int mainStartSamples_ = 0;

    int burstCount_ = 0;
    std::array<int, 5> burstStartSamples_{};
    std::array<float, 5> burstAmplitudes_{};
    std::array<float, 5> burstPans_{};

    std::array<float, 8> partialFrequencies_{};
    std::array<float, 8> partialPhases_{};

    BiquadState noiseHPF_L_{};
    BiquadState noiseHPF_R_{};
    BiquadState noiseLPF_L_{};
    BiquadState noiseLPF_R_{};

    // Filter cache to avoid recalculating biquad trig every sample
    float lastHPF_ = -1.0f;
    float lastLPF_ = -1.0f;
    float b0_h_ = 0.0f, b1_h_ = 0.0f, b2_h_ = 0.0f, a1_h_ = 0.0f, a2_h_ = 0.0f;
    float b0_l_ = 0.0f, b1_l_ = 0.0f, b2_l_ = 0.0f, a1_l_ = 0.0f, a2_l_ = 0.0f;

    // PRNG seed for deterministic and jittered noise
    uint32_t prngState_ = 0x12345678;
    float pinkB0_ = 0.0f, pinkB1_ = 0.0f, pinkB2_ = 0.0f;
    float lofiHeld_ = 0.0f;
    int lofiCounter_ = 0;
};

} // namespace openclapper::core
