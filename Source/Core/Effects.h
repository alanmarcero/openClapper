#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <vector>

namespace openclapper::core {

[[nodiscard]] inline double logCosh(double u) noexcept {
    const double absU = std::abs(u);
    if (absU > 20.0) {
        return absU - std::numbers::ln2;
    }
    return absU + std::log1p(std::exp(-2.0 * absU)) - std::numbers::ln2;
}

[[nodiscard]] inline double adaaTanh1(double u, double& prevU, double& prevF1) noexcept {
    const double clamped = std::clamp(u, -24.0, 24.0);
    const double f1 = logCosh(clamped);
    const double du = clamped - prevU;
    const double mid = 0.5 * (clamped + prevU);
    const double prevIntegral = prevF1;
    prevU = clamped;
    prevF1 = f1;
    if (std::abs(du) > 1.0e-5) {
        return (f1 - prevIntegral) / du;
    }
    return std::tanh(mid);
}

// 2-branch polyphase IIR half-band 2x oversampler with matched branch group delay.
struct PolyphaseHalfband2x {
    static constexpr double kAp0A = 0.07986642623635751;
    static constexpr double kAp0B = 0.54535365107113220;
    static constexpr double kAp1A = 0.28382934487410993;
    static constexpr double kAp1B = 0.83441189148073790;

    struct Allpass1 {
        double state = 0.0;
        double prevIn = 0.0;

        void reset() noexcept {
            state = 0.0;
            prevIn = 0.0;
        }

        [[nodiscard]] double process(double x, double coeff) noexcept {
            const double y = coeff * (x - state) + prevIn;
            prevIn = x;
            state = y;
            return y;
        }
    };

    Allpass1 up0a{}, up0b{}, up1a{}, up1b{};
    Allpass1 dn0a{}, dn0b{}, dn1a{}, dn1b{};

    void reset() noexcept {
        up0a.reset(); up0b.reset(); up1a.reset(); up1b.reset();
        dn0a.reset(); dn0b.reset(); dn1a.reset(); dn1b.reset();
    }

    [[nodiscard]] std::array<double, 2> upsample(double x) noexcept {
        return {
            up0b.process(up0a.process(x, kAp0A), kAp0B),
            up1b.process(up1a.process(x, kAp1A), kAp1B)
        };
    }

    [[nodiscard]] double downsample(double y0, double y1) noexcept {
        const double dn0 = dn0b.process(dn0a.process(y1, kAp0A), kAp0B);
        const double dn1 = dn1b.process(dn1a.process(y0, kAp1A), kAp1B);
        return 0.5 * (dn0 + dn1);
    }
};

[[nodiscard]] inline float readHermiteCircular(const std::vector<float>& buffer, double delaySamples, size_t writePos) noexcept {
    const int bufSize = static_cast<int>(buffer.size());
    const double size = static_cast<double>(bufSize);
    double readPos = static_cast<double>(writePos) - delaySamples;
    while (readPos < 0.0) readPos += size;
    while (readPos >= size) readPos -= size;

    const int i1 = static_cast<int>(readPos);
    const float frac = static_cast<float>(readPos - static_cast<double>(i1));
    const int i0 = (i1 - 1 + bufSize) % bufSize;
    const int i2 = (i1 + 1) % bufSize;
    const int i3 = (i1 + 2) % bufSize;

    const float y0 = buffer[static_cast<size_t>(i0)];
    const float y1 = buffer[static_cast<size_t>(i1)];
    const float y2 = buffer[static_cast<size_t>(i2)];
    const float y3 = buffer[static_cast<size_t>(i3)];

    const float c0 = y1;
    const float c1 = 0.5f * (y2 - y0);
    const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    return ((c3 * frac + c2) * frac + c1) * frac + c0;
}

enum class DistortionType {
    Tape = 0,
    Tube = 1,
    HardClip = 2,
    Wavefold = 3
};

enum class ReverseSyncMode {
    Off = 0,
    BeatEnd = 1,
    Beat4Auto = 2,
    HalfBeat = 3,
    QuarterBeat = 4
};

enum class SlapbackDivision {
    Off = 0,
    Sixteenth = 1,
    DottedSixteenth = 2,
    Eighth = 3,
    DottedEighth = 4,
    Quarter = 5
};

class SlapbackDelay {
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    bool process(float& left, float& right, SlapbackDivision division, float mix, double bpm, bool trigger = false, float feedback = 0.0f) noexcept;

private:
    struct FeedbackBiquad {
        double z1 = 0.0;
        double z2 = 0.0;

        void reset() noexcept { z1 = z2 = 0.0; }

        [[nodiscard]] float process(float x, double b0, double b1, double b2, double a1, double a2) noexcept {
            const double in = static_cast<double>(x);
            const double out = b0 * in + z1;
            z1 = b1 * in - a1 * out + z2;
            z2 = b2 * in - a2 * out;
            return static_cast<float>(out);
        }
    };

    double sampleRate_ = 48000.0;
    double smoothedDelaySamples_ = 0.0;
    double glideCoeff_ = 0.001;
    double hpfB0_ = 1.0, hpfB1_ = 0.0, hpfB2_ = 0.0, hpfA1_ = 0.0, hpfA2_ = 0.0;
    double lpfB0_ = 1.0, lpfB1_ = 0.0, lpfB2_ = 0.0, lpfA1_ = 0.0, lpfA2_ = 0.0;
    FeedbackBiquad hpfL_{}, hpfR_{}, lpfL_{}, lpfR_{};
    double adaaPrevU_L_ = 0.0, adaaPrevF1_L_ = 0.0;
    double adaaPrevU_R_ = 0.0, adaaPrevF1_R_ = 0.0;
    size_t writeIndex_ = 0;
    std::vector<float> leftBuffer_;
    std::vector<float> rightBuffer_;
    std::vector<float> triggerBuffer_;
    bool delayTriggerActive_ = false;
    bool delayInitialized_ = false;
};

// ============================================================================
// Distortion / Impact Shaper
// ============================================================================

class DistortionShaper {
public:
    void reset() noexcept;
    [[nodiscard]] float processSample(float input, DistortionType type, float drive, float tone, double sampleRate) noexcept;

private:
    PolyphaseHalfband2x stage1_{};
    PolyphaseHalfband2x stage2_{};
    double u1_ = 0.0;
    double u2_ = 0.0;
    double f1Prev1_ = 0.0;
    double f2Prev1_ = 0.0;
    double f2Prev2_ = 0.0;
    int lastType_ = -1;
    bool primed_ = false;
    float toneFilterState_ = 0.0f;
    double dcX_ = 0.0;
    double dcY_ = 0.0;
};

// ============================================================================
// Stereo Compressor
// ============================================================================

struct CompressorSettings {
    float thresholdDb = -14.0f; // -40..0 dB
    float ratio = 4.0f;         // 1..20
    float attackMs = 5.0f;      // 0.1..50 ms
    float releaseMs = 80.0f;    // 10..500 ms
    float makeupDb = 3.0f;      // 0..24 dB
    float mix = 1.0f;           // 0..1

    bool operator==(const CompressorSettings&) const noexcept = default;
};

class StereoCompressor {
public:
    void prepare(double sampleRate);
    void reset() noexcept;
    void process(float& left, float& right, const CompressorSettings& settings) noexcept;

private:
    double sampleRate_ = 48000.0;
    float envelopeDb_ = -100.0f;
};

// ============================================================================
// Stereo 3-Band EQ (Low Shelf, Mid Peak, High Shelf)
// ============================================================================

struct ThreeBandEqSettings {
    float lowFreqHz = 80.0f;
    float lowGainDb = 0.0f;
    float midFreqHz = 1000.0f;
    float midGainDb = 0.0f;
    float highFreqHz = 7500.0f;
    float highGainDb = 0.0f;

    bool operator==(const ThreeBandEqSettings&) const noexcept = default;
};

/** Stereo 3-band EQ with smooth log-frequency glide and +/-12 dB cut/boost per band. */
class ThreeBandEq {
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void process(float& left, float& right, const ThreeBandEqSettings& settings) noexcept;

private:
    enum class BandType { lowShelf, peak, highShelf };
    struct Band {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
        double z1L = 0.0, z2L = 0.0, z1R = 0.0, z2R = 0.0;
        float smoothedLogFreq = 0.0f;
        float smoothedDb = 0.0f;
        bool initialized = false;
    };

    void updateBand(Band& band, BandType type, float minHz, float maxHz, float targetHz, float targetDb) noexcept;
    static void filterBand(Band& band, float& left, float& right) noexcept;

    double sampleRate_ = 48000.0;
    float glideCoeff_ = 0.001f;
    std::array<Band, 3> bands_{};
};

// ============================================================================
// Brickwall Limiter
// ============================================================================

struct LimiterSettings {
    float ceilingDb = -0.3f;    // -12..0 dB
    float releaseMs = 20.0f;    // 5..100 ms
    bool enabled = true;

    bool operator==(const LimiterSettings&) const noexcept = default;
};

class BrickwallLimiter {
public:
    static constexpr int kLookaheadSamples = 64;

    BrickwallLimiter() noexcept { reset(); }

    void prepare(double sampleRate);
    void reset() noexcept;
    void process(float& left, float& right, const LimiterSettings& settings) noexcept;

private:
    double sampleRate_ = 48000.0;
    std::array<float, kLookaheadSamples> delayL_{};
    std::array<float, kLookaheadSamples> delayR_{};
    std::array<float, 8> tpHistL_{};
    std::array<float, 8> tpHistR_{};
    std::array<float, 18> scDelay_{};
    std::array<float, 49> minWindow_{};
    std::array<float, 40> smoothBuf_{};
    size_t writePos_ = 0;
    size_t tpPos_ = 0;
    size_t scPos_ = 0;
    size_t minPos_ = 0;
    size_t smoothPos_ = 0;
    float envGain_ = 1.0f;
};

// ============================================================================
// Host-Synced Reverse Clap Swell Helper
// ============================================================================

[[nodiscard]] bool shouldTriggerReverse(
    ReverseSyncMode mode,
    bool manualCue,
    double ppqPosition,
    double beatsPerBar) noexcept;

[[nodiscard]] double reverseTargetPpq(
    ReverseSyncMode mode,
    double ppqPosition,
    double beatsPerBar) noexcept;

[[nodiscard]] int computeSyncedReverseSamples(
    ReverseSyncMode mode,
    double ppqPosition,
    double bpm,
    double beatsPerBar,
    double sampleRate) noexcept;

} // namespace openclapper::core
