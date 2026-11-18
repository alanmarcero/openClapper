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

enum class ClickType {
    Needle = 0,
    Crack = 1,
    Thump = 2
};

struct LayerSettings {
    float velocityResponse = 1.0f;
    bool mute = false;
    bool solo = false;
    bool operator==(const LayerSettings&) const noexcept = default;
};

struct ClapSynthSettings {
    // Body, noise, transient, pre-clap; these indices also define auxiliary output order.
    std::array<LayerSettings, 4> layers{};
    bool ampEnabled = false;
    float ampAttackMs = 0.0f;
    float ampDecayMs = 100.0f;
    float ampSustain = 0.8f;
    float ampReleaseMs = 200.0f;

    // Pre-Clap Flam (Micro-bursts)
    int preClapCount = 3;         // 1..5 micro-bursts before main slap
    float preClapSpreadMs = 12.0f;// 2..45 ms spacing between bursts
    float preClapDecayMs = 8.0f;  // 2..35 ms decay of each burst
    float preClapJitter = 0.15f;  // 0..1 timing and pitch humanization
    float preClapFlamSpread = 0.45f; // 0..1 stereo micro-panning spread
    float preClapLevel = 0.85f;   // 0..1 volume of pre-claps

    // Resonant Body / Tone & Impact Slam
    float toneQ = 3.2f;           // 0.5..12.0 resonance
    float toneDecayMs = 160.0f;   // 15..1200 ms tone decay
    float tonePitchEnv = 0.0f;    // -24..+24 semitones, decays to the MIDI note
    float tonePitchDecayMs = 28.0f; // 3..600 ms pitch sweep time
    float toneHPF = 20.0f;        // 20..12000 Hz body high-pass
    float toneLPF = 20000.0f;     // 100..20000 Hz body low-pass
    float toneLevel = 0.75f;      // 0..1 level of tone core
    float impactPunch = 0.45f;    // 0..1 low-mid chest slam & multi-hand slap density

    // Noise & Sizzle
    NoiseType noiseType = NoiseType::White;
    float noiseAttackMs = 0.0f;   // 0..20 ms
    float noiseHoldMs = 12.0f;    // 0..90 ms
    float noiseDecayMs = 320.0f;  // 20..2200 ms
    float noiseCurve = -0.25f;    // -1..+1 shape curve (-1 exp, 0 lin, +1 sustained shoulder)
    float noiseHPF = 350.0f;      // 20..2500 Hz
    float noiseLPF = 9500.0f;     // 800..20000 Hz
    float noiseStereoWidth = 1.0f;// 0 (mono) .. 2 (ultra-wide)
    float noiseLevel = 0.85f;     // 0..1

    // Click / Transient
    ClickType clickType = ClickType::Needle;
    float clickLevel = 0.4f;      // 0..1
    float clickFreqHz = 4500.0f;  // 1000..12000 Hz
    float clickDecayMs = 0.8f;    // 0.2..20 ms transient time constant
    float pitchHumanizeCents = 0.0f; // 0..100 cents per-hit pitch variation

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

/** Release time of the room gate, shared with the reported plugin tail. */
inline constexpr float kReverbGateReleaseMs = 64.0f;

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
    void noteOff(const ClapSynthSettings& settings) noexcept;
    [[nodiscard]] StereoFrame processSample(const ClapSynthSettings& settings, bool* mainHitStarted = nullptr,
                                            std::array<StereoFrame, 4>* components = nullptr) noexcept;
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
    [[nodiscard]] StereoFrame generateNoise(NoiseType type) noexcept;

    double sampleRate_ = 48000.0;
    bool isPlaying_ = false;
    float velocity_ = 1.0f;
    float pitchHumanizeRatio_ = 1.0f;
    int midiNote_ = 69;
    uint32_t triggerCount_ = 0;

    int64_t totalSamplesPlayed_ = 0;
    bool ampEnabled_ = false;
    float amplitude_ = 0.0f;
    float releaseStart_ = 0.0f;
    int releaseSamples_ = 0;
    int releaseAge_ = -1;
    int totalDurationSamples_ = 0;
    int mainStartSamples_ = 0;

    int burstCount_ = 0;
    std::array<int, 5> burstStartSamples_{};
    std::array<float, 5> burstAmplitudes_{};
    std::array<float, 5> burstPans_{};

    BiquadState toneFilterL_{};
    BiquadState toneBloomFilterL_{};
    BiquadState toneHPF_{};
    BiquadState toneLPF_{};
    BiquadState clickFilter_{};
    float lastToneFrequency_ = -1.0f;
    float lastToneQ_ = -1.0f;
    float lastClickFrequency_ = -1.0f;
    float lastClickQ_ = -1.0f;
    float b0_t_ = 0.0f, b1_t_ = 0.0f, b2_t_ = 0.0f, a1_t_ = 0.0f, a2_t_ = 0.0f;
    float b0_tb_ = 0.0f, b1_tb_ = 0.0f, b2_tb_ = 0.0f, a1_tb_ = 0.0f, a2_tb_ = 0.0f;
    float b0_c_ = 0.0f, b1_c_ = 0.0f, b2_c_ = 0.0f, a1_c_ = 0.0f, a2_c_ = 0.0f;
    float impactLp_ = 0.0f;
    float lastToneHPF_ = -1.0f, lastToneLPF_ = -1.0f;
    float b0_th_ = 0.0f, b1_th_ = 0.0f, b2_th_ = 0.0f, a1_th_ = 0.0f, a2_th_ = 0.0f;
    float b0_tl_ = 1.0f, b1_tl_ = 0.0f, b2_tl_ = 0.0f, a1_tl_ = 0.0f, a2_tl_ = 0.0f;

    BiquadState preHPF_L_{}, preHPF_R_{}, preLPF_L_{}, preLPF_R_{};
    BiquadState preHPF2_L_{}, preHPF2_R_{}, preLPF2_L_{}, preLPF2_R_{};
    BiquadState noiseHPF_L_{};
    BiquadState noiseHPF_R_{};
    BiquadState noiseLPF_L_{};
    BiquadState noiseLPF_R_{};
    BiquadState noiseHPF2_L_{};
    BiquadState noiseHPF2_R_{};
    BiquadState noiseLPF2_L_{};
    BiquadState noiseLPF2_R_{};

    // Filter cache to avoid recalculating biquad trig every sample
    float lastHPF_ = -1.0f;
    float lastLPF_ = -1.0f;
    float b0_h_ = 0.0f, b1_h_ = 0.0f, b2_h_ = 0.0f, a1_h_ = 0.0f, a2_h_ = 0.0f;
    float b0_l_ = 0.0f, b1_l_ = 0.0f, b2_l_ = 0.0f, a1_l_ = 0.0f, a2_l_ = 0.0f;

    // PRNG seed for deterministic and jittered noise
    uint32_t prngState_ = 0x12345678;
    std::array<float, 3> pinkPoles_{}, pinkGains_{}, pinkLeft_{}, pinkRight_{};
    StereoFrame lofiHeld_{};
    double lofiPhase_ = 1.0, lofiStep_ = 0.25;
    float contactRiseSteps_ = 1.0f;
};

} // namespace openclapper::core
