#include "Effects.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace openclapper::core {

namespace {

[[nodiscard]] constexpr float pi() noexcept {
    return std::numbers::pi_v<float>;
}

[[nodiscard]] inline float dbToLin(float db) noexcept {
    return std::pow(10.0f, db / 20.0f);
}

[[nodiscard]] inline float linToDb(float lin) noexcept {
    return 20.0f * std::log10(std::max(lin, 1e-6f));
}

} // namespace

// ============================================================================
// DistortionShaper
// ============================================================================

void DistortionShaper::reset() noexcept {
    toneFilterState_ = 0.0f;
}

float DistortionShaper::processSample(
    float input,
    DistortionType type,
    float drive,
    float tone,
    double sampleRate) noexcept {
    if (drive <= 0.001f) {
        return input;
    }

    // Drive adds up to +28 dB input gain
    const float gain = dbToLin(drive * 28.0f);
    const float x = input * gain;
    float shaped = 0.0f;

    switch (type) {
        case DistortionType::Tape: {
            // Warm asymmetric soft tape saturation
            shaped = (std::tanh(x * 1.4f + 0.08f) - std::tanh(0.08f)) * 0.85f;
            break;
        }
        case DistortionType::Tube: {
            // Asymmetric triode tube saturation with even harmonics
            if (x >= 0.0f) {
                shaped = x / (1.0f + x);
            } else {
                shaped = x / (1.0f - 0.45f * x);
            }
            shaped *= 1.3f;
            break;
        }
        case DistortionType::HardClip: {
            shaped = std::clamp(x * 1.5f, -1.0f, 1.0f);
            break;
        }
        case DistortionType::Wavefold: {
            shaped = std::sin(x * pi() * 0.7f);
            break;
        }
    }

    // Level-normalize according to drive without collapsing punch
    float compGain = 1.0f;
    if (type == DistortionType::HardClip) {
        compGain = 1.0f / (1.0f + drive * 0.35f);
    } else if (type == DistortionType::Wavefold) {
        compGain = 1.0f / (1.0f + drive * 0.45f);
    } else {
        compGain = 1.0f / (1.0f + drive * 0.8f);
    }
    float output = shaped * compGain;

    // Post-distortion tone tilt filter
    if (std::abs(tone) > 0.01f) {
        const float alpha = std::clamp(std::abs(tone) * 0.5f, 0.01f, 0.9f);
        toneFilterState_ = toneFilterState_ + alpha * (output - toneFilterState_);
        if (tone < 0.0f) {
            // Darker tone: lowpass
            output = toneFilterState_;
        } else {
            // Brighter tone: high shelf emphasis
            output = output + (output - toneFilterState_) * (tone * 1.5f);
        }
    }

    return output;
}

// ============================================================================
// StereoCompressor
// ============================================================================

void StereoCompressor::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
    reset();
}

void StereoCompressor::reset() noexcept {
    envelopeDb_ = -100.0f;
}

void StereoCompressor::process(float& left, float& right, const CompressorSettings& settings) noexcept {
    const float dryL = left;
    const float dryR = right;

    // Peak sidechain level detector
    const float peak = std::max(std::abs(left), std::abs(right));
    const float inputDb = linToDb(peak);

    // Attack and release coefficients
    const float attackCoeff = std::exp(-1.0f / (std::max(0.1f, settings.attackMs) * 0.001f * static_cast<float>(sampleRate_)));
    const float releaseCoeff = std::exp(-1.0f / (std::max(1.0f, settings.releaseMs) * 0.001f * static_cast<float>(sampleRate_)));

    if (inputDb > envelopeDb_) {
        envelopeDb_ = inputDb + attackCoeff * (envelopeDb_ - inputDb);
    } else {
        envelopeDb_ = inputDb + releaseCoeff * (envelopeDb_ - inputDb);
    }

    // Gain reduction calculation
    float gainDb = 0.0f;
    if (envelopeDb_ > settings.thresholdDb && settings.ratio > 1.0f) {
        const float overDb = envelopeDb_ - settings.thresholdDb;
        gainDb = -overDb * (1.0f - (1.0f / settings.ratio));
    }

    const float totalGainLin = dbToLin(gainDb + settings.makeupDb);
    const float wetL = left * totalGainLin;
    const float wetR = right * totalGainLin;

    left = dryL * (1.0f - settings.mix) + wetL * settings.mix;
    right = dryR * (1.0f - settings.mix) + wetR * settings.mix;
}

// ============================================================================
// BrickwallLimiter
// ============================================================================

void BrickwallLimiter::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
    reset();
}

void BrickwallLimiter::reset() noexcept {
    gain_ = 1.0f;
}

void BrickwallLimiter::process(float& left, float& right, const LimiterSettings& settings) noexcept {
    if (!settings.enabled) {
        return;
    }

    const float ceiling = dbToLin(settings.ceilingDb);
    const float releaseCoeff = std::exp(-1.0f / (std::max(1.0f, settings.releaseMs) * 0.001f * static_cast<float>(sampleRate_)));

    const float peak = std::max(std::abs(left), std::abs(right));

    // Instant attack look-ahead / catch
    if (peak * gain_ > ceiling && peak > 1e-6f) {
        gain_ = ceiling / peak;
    } else {
        gain_ = 1.0f + releaseCoeff * (gain_ - 1.0f);
    }

    left *= gain_;
    right *= gain_;

    // Safety soft saturation knee right at the ceiling to guarantee 0.0 overshoot
    const float thresholdSoft = ceiling * 0.95f;
    if (std::abs(left) > thresholdSoft) {
        const float sign = left > 0.0f ? 1.0f : -1.0f;
        const float diff = std::abs(left) - thresholdSoft;
        left = sign * (thresholdSoft + (ceiling - thresholdSoft) * std::tanh(diff / (ceiling - thresholdSoft)));
    }
    if (std::abs(right) > thresholdSoft) {
        const float sign = right > 0.0f ? 1.0f : -1.0f;
        const float diff = std::abs(right) - thresholdSoft;
        right = sign * (thresholdSoft + (ceiling - thresholdSoft) * std::tanh(diff / (ceiling - thresholdSoft)));
    }
}

// ============================================================================
// Host-Synced Reverse Clap Swell Helper
// ============================================================================

bool shouldTriggerReverse(
    ReverseSyncMode mode,
    bool manualCue,
    double ppqPosition,
    double beatsPerBar) noexcept {
    if (mode == ReverseSyncMode::Off) {
        return manualCue;
    }
    if (mode == ReverseSyncMode::Beat4Auto) {
        const double barBeats = beatsPerBar > 0.0 ? beatsPerBar : 4.0;
        const double beatInBar = std::fmod(ppqPosition, barBeats);
        return beatInBar >= (barBeats - 1.0 - 0.05);
    }
    return true;
}

double reverseTargetPpq(
    ReverseSyncMode mode,
    double ppqPosition,
    double beatsPerBar) noexcept {
    const double barBeats = beatsPerBar > 0.0 ? beatsPerBar : 4.0;
    switch (mode) {
        case ReverseSyncMode::Beat4Auto: {
            const double currentBar = std::floor(ppqPosition / barBeats);
            return (currentBar + 1.0) * barBeats;
        }
        case ReverseSyncMode::BeatEnd:
            return std::floor(ppqPosition) + 1.0;
        case ReverseSyncMode::HalfBeat:
            return (std::floor(ppqPosition * 2.0) + 1.0) / 2.0;
        case ReverseSyncMode::QuarterBeat:
            return (std::floor(ppqPosition * 4.0) + 1.0) / 4.0;
        case ReverseSyncMode::Off:
        default:
            return ppqPosition + 1.0;
    }
}

int computeSyncedReverseSamples(
    ReverseSyncMode mode,
    double ppqPosition,
    double bpm,
    double beatsPerBar,
    double sampleRate) noexcept {
    if (mode == ReverseSyncMode::Off || bpm <= 0.0) {
        return static_cast<int>(sampleRate * 0.45); // Default 450 ms
    }
    const double targetPpq = reverseTargetPpq(mode, ppqPosition, beatsPerBar);
    const double beatsUntilTarget = std::max(0.01, targetPpq - ppqPosition);
    const double secondsUntilTarget = beatsUntilTarget * (60.0 / bpm);
    return std::max(32, static_cast<int>(secondsUntilTarget * sampleRate));
}

} // namespace openclapper::core
