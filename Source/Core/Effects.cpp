#include "Effects.h"
#include <algorithm>
#include <cmath>
#include <numbers>
#include <ranges>

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

void SlapbackDelay::prepare(double sampleRate) {
    sampleRate_ = std::max(1.0, sampleRate);
    glideCoeff_ = 1.0 - std::exp(-1.0 / std::max(1.0, 0.030 * sampleRate_));

    const double wHp = 2.0 * std::numbers::pi * std::min(150.0, 0.45 * sampleRate_) / sampleRate_;
    const double alphaHp = std::sin(wHp) * 0.5 * std::numbers::sqrt2;
    const double csHp = std::cos(wHp);
    const double a0Hp = 1.0 + alphaHp;
    hpfB0_ = ((1.0 + csHp) * 0.5) / a0Hp;
    hpfB1_ = -(1.0 + csHp) / a0Hp;
    hpfB2_ = hpfB0_;
    hpfA1_ = (-2.0 * csHp) / a0Hp;
    hpfA2_ = (1.0 - alphaHp) / a0Hp;

    const double wLp = 2.0 * std::numbers::pi * std::min(6500.0, 0.45 * sampleRate_) / sampleRate_;
    const double alphaLp = std::sin(wLp) * 0.5 * std::numbers::sqrt2;
    const double csLp = std::cos(wLp);
    const double a0Lp = 1.0 + alphaLp;
    lpfB0_ = ((1.0 - csLp) * 0.5) / a0Lp;
    lpfB1_ = (1.0 - csLp) / a0Lp;
    lpfB2_ = lpfB0_;
    lpfA1_ = (-2.0 * csLp) / a0Lp;
    lpfA2_ = (1.0 - alphaLp) / a0Lp;

    constexpr double maxDelaySeconds = 4.0;
    const auto capacity = static_cast<size_t>(std::ceil(sampleRate_ * maxDelaySeconds)) + 8;
    leftBuffer_.assign(capacity, 0.0f);
    rightBuffer_.assign(capacity, 0.0f);
    triggerBuffer_.assign(capacity, 0.0f);
    reset();
}

void SlapbackDelay::reset() noexcept {
    std::fill(leftBuffer_.begin(), leftBuffer_.end(), 0.0f);
    std::fill(rightBuffer_.begin(), rightBuffer_.end(), 0.0f);
    std::fill(triggerBuffer_.begin(), triggerBuffer_.end(), 0.0f);
    writeIndex_ = 0;
    smoothedDelaySamples_ = 0.0;
    hpfL_.reset();
    hpfR_.reset();
    lpfL_.reset();
    lpfR_.reset();
    adaaPrevU_L_ = adaaPrevF1_L_ = 0.0;
    adaaPrevU_R_ = adaaPrevF1_R_ = 0.0;
    delayTriggerActive_ = false;
    delayInitialized_ = false;
}

bool SlapbackDelay::process(
    float& left,
    float& right,
    SlapbackDivision division,
    float mix,
    double bpm,
    bool trigger,
    float feedback) noexcept {
    if (leftBuffer_.empty() || rightBuffer_.empty()) {
        return false;
    }

    const float dryLeft = left;
    const float dryRight = right;
    constexpr std::array<double, 6> beatFractions = { 0.0, 0.25, 0.375, 0.5, 0.75, 1.0 };
    const auto divisionIndex = static_cast<size_t>(division);

    if (divisionIndex == 0 || divisionIndex >= beatFractions.size() || mix <= 0.0f) {
        delayTriggerActive_ = false;
        delayInitialized_ = false;
        adaaPrevU_L_ = adaaPrevF1_L_ = 0.0;
        adaaPrevU_R_ = adaaPrevF1_R_ = 0.0;
        leftBuffer_[writeIndex_] = dryLeft;
        rightBuffer_[writeIndex_] = dryRight;
        triggerBuffer_[writeIndex_] = trigger ? 1.0f : 0.0f;
        writeIndex_ = (writeIndex_ + 1) % leftBuffer_.size();
        return false;
    }

    const double safeBpm = std::isfinite(bpm) && bpm > 0.0 ? bpm : 138.0;
    const double targetDelay = std::clamp(
        beatFractions[divisionIndex] * 60.0 / safeBpm * sampleRate_,
        2.0,
        static_cast<double>(leftBuffer_.size() - 4));
    if (!delayInitialized_) {
        smoothedDelaySamples_ = targetDelay;
        delayInitialized_ = true;
    }
    smoothedDelaySamples_ += glideCoeff_ * (targetDelay - smoothedDelaySamples_);

    const float delayedLeft = readHermiteCircular(leftBuffer_, smoothedDelaySamples_, writeIndex_);
    const float delayedRight = readHermiteCircular(rightBuffer_, smoothedDelaySamples_, writeIndex_);

    double markerPos = static_cast<double>(writeIndex_) - smoothedDelaySamples_;
    if (markerPos < 0.0) {
        markerPos += static_cast<double>(triggerBuffer_.size());
    }
    const auto firstIndex = static_cast<size_t>(markerPos) % triggerBuffer_.size();
    const auto secondIndex = (firstIndex + 1) % triggerBuffer_.size();
    const float fraction = static_cast<float>(markerPos - std::floor(markerPos));
    const float delayedMarker = triggerBuffer_[firstIndex] * (1.0f - fraction) + triggerBuffer_[secondIndex] * fraction;
    const bool delayedTrigger = delayedMarker > 0.0f && !delayTriggerActive_;
    delayTriggerActive_ = delayedMarker > 0.0f;

    // Bandpass + ADAA inside the feedback loop keeps the first slap full-bandwidth while taming repeat buildup.
    const float clampedFeedback = std::clamp(feedback, 0.0f, 0.85f);
    const float bpLeft = lpfL_.process(
        hpfL_.process(delayedLeft * clampedFeedback, hpfB0_, hpfB1_, hpfB2_, hpfA1_, hpfA2_),
        lpfB0_, lpfB1_, lpfB2_, lpfA1_, lpfA2_);
    const float bpRight = lpfR_.process(
        hpfR_.process(delayedRight * clampedFeedback, hpfB0_, hpfB1_, hpfB2_, hpfA1_, hpfA2_),
        lpfB0_, lpfB1_, lpfB2_, lpfA1_, lpfA2_);
    const float fbLeft = static_cast<float>(adaaTanh1(static_cast<double>(bpLeft), adaaPrevU_L_, adaaPrevF1_L_));
    const float fbRight = static_cast<float>(adaaTanh1(static_cast<double>(bpRight), adaaPrevU_R_, adaaPrevF1_R_));

    leftBuffer_[writeIndex_] = std::clamp(dryLeft + fbLeft, -4.0f, 4.0f);
    rightBuffer_[writeIndex_] = std::clamp(dryRight + fbRight, -4.0f, 4.0f);
    triggerBuffer_[writeIndex_] = trigger ? 1.0f : 0.0f;
    writeIndex_ = (writeIndex_ + 1) % leftBuffer_.size();

    const float wet = std::clamp(mix, 0.0f, 1.0f);
    left = dryLeft * (1.0f - wet) + delayedLeft * wet;
    right = dryRight * (1.0f - wet) + delayedRight * wet;
    return delayedTrigger;
}

// ============================================================================
// DistortionShaper
// ============================================================================

namespace {

struct NonlinearEval {
    double f0 = 0.0;
    double f1 = 0.0;
    double f2 = 0.0;
};

[[nodiscard]] inline NonlinearEval evalCurve(DistortionType type, double u) noexcept {
    if (type == DistortionType::Tape) {
        const double c = std::clamp(u, -18.0, 18.0);
        const double t = std::tanh(c);
        const double lc = logCosh(c);
        const double w = std::abs(c);
        const double p = 1.0 / (1.0 + std::exp(2.0 * w));
        const double lnOneMinusP = -std::log1p(std::exp(-2.0 * w));
        double li2P = 0.0;
        std::ranges::for_each(std::views::iota(0, 28), [&](int idx) {
            const int k = 28 - idx;
            li2P = p * ((1.0 / static_cast<double>(k * k)) + li2P);
        });
        constexpr double kR0 = (std::numbers::pi * std::numbers::pi) / 24.0;
        const double rW = 0.5 * li2P + 0.25 * lnOneMinusP * lnOneMinusP;
        const double gAbs = 0.5 * w * w - w * std::numbers::ln2 + (kR0 - rW);
        const double sign = c >= 0.0 ? 1.0 : -1.0;
        return {
            t + 0.18 * t * t,
            0.18 * c + lc - 0.18 * t,
            0.09 * c * c + sign * gAbs - 0.18 * lc
        };
    }

    if (type == DistortionType::Tube) {
        const double c = std::clamp(u, -40.0, 40.0);
        if (c >= 0.0) {
            const double ln1p = std::log1p(c);
            return {
                1.3 * (c / (1.0 + c)),
                1.3 * (c - ln1p),
                1.3 * (0.5 * c * c - (1.0 + c) * ln1p + c)
            };
        }
        constexpr double a = 0.45;
        constexpr double scale = 1.3 / a;
        constexpr double invA = 1.0 / a;
        constexpr double invA2 = 1.0 / (a * a);
        const double oneMinusAx = 1.0 - a * c;
        const double lnTerm = std::log(oneMinusAx);
        return {
            1.3 * (c / oneMinusAx),
            scale * (-c - invA * lnTerm),
            scale * (-0.5 * c * c + invA2 * (oneMinusAx * lnTerm + a * c))
        };
    }

    if (type == DistortionType::HardClip) {
        if (u > 1.0) {
            return { 1.0, u - 0.5, 0.5 * u * u - 0.5 * u + (1.0 / 6.0) };
        }
        if (u < -1.0) {
            return { -1.0, -u - 0.5, -0.5 * u * u - 0.5 * u - (1.0 / 6.0) };
        }
        return { u, 0.5 * u * u, (1.0 / 6.0) * u * u * u };
    }

    const double c = std::clamp(u, -64.0, 64.0);
    const double s = std::sin(c);
    return { s, 1.0 - std::cos(c), c - s };
}

} // namespace

void DistortionShaper::reset() noexcept {
    stage1_.reset();
    stage2_.reset();
    u1_ = u2_ = 0.0;
    f1Prev1_ = f2Prev1_ = f2Prev2_ = 0.0;
    lastType_ = -1;
    primed_ = false;
    toneFilterState_ = 0.0f;
    dcX_ = 0.0;
    dcY_ = 0.0;
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

    if (static_cast<int>(type) != lastType_) {
        lastType_ = static_cast<int>(type);
        primed_ = false;
    }

    const double gain = static_cast<double>(dbToLin(drive * 28.0f));
    double preScale = 1.0;
    if (type == DistortionType::Tape) preScale = 1.2;
    if (type == DistortionType::HardClip) preScale = 1.5;
    if (type == DistortionType::Wavefold) preScale = 0.7 * std::numbers::pi;
    const double totalScale = gain * preScale;

    // 4x polyphase half-band oversampling + singularity-free ADAA2 suppresses even/odd harmonic foldback.
    const auto [s2_0, s2_1] = stage1_.upsample(static_cast<double>(input));
    const auto [s4_0, s4_1] = stage2_.upsample(s2_0);
    const auto [s4_2, s4_3] = stage2_.upsample(s2_1);
    const std::array<double, 4> u4{
        s4_0 * totalScale,
        s4_1 * totalScale,
        s4_2 * totalScale,
        s4_3 * totalScale
    };

    if (!primed_) {
        const auto init = evalCurve(type, u4[0]);
        u1_ = u2_ = u4[0];
        f1Prev1_ = init.f1;
        f2Prev1_ = f2Prev2_ = init.f2;
        primed_ = true;
    }

    const auto halfHat = [&](double a, double f1A, double f2A, double b, double f2B) noexcept {
        const double d = b - a;
        if (std::abs(d) > 1.0e-4) {
            return (f2B - f2A - d * f1A) / (d * d);
        }
        return 0.5 * evalCurve(type, (2.0 * a + b) * (1.0 / 3.0)).f0;
    };

    std::array<double, 4> y4{};
    std::ranges::for_each(std::views::iota(size_t{ 0 }, size_t{ 4 }), [&](size_t i) {
        const double u0 = u4[i];
        const auto cur = evalCurve(type, u0);
        y4[i] = halfHat(u1_, f1Prev1_, f2Prev1_, u0, cur.f2)
              + halfHat(u1_, f1Prev1_, f2Prev1_, u2_, f2Prev2_);
        u2_ = u1_;
        f2Prev2_ = f2Prev1_;
        u1_ = u0;
        f1Prev1_ = cur.f1;
        f2Prev1_ = cur.f2;
    });

    const double d2_0 = stage2_.downsample(y4[0], y4[1]);
    const double d2_1 = stage2_.downsample(y4[2], y4[3]);
    const float shaped = static_cast<float>(stage1_.downsample(d2_0, d2_1));

    float compGain = 1.0f / (1.0f + drive * 0.8f);
    if (type == DistortionType::HardClip) {
        compGain = 1.0f / (1.0f + drive * 0.35f);
    }
    if (type == DistortionType::Wavefold) {
        compGain = 1.0f / (1.0f + drive * 0.45f);
    }
    float output = shaped * compGain;

    if (std::abs(tone) > 0.01f) {
        const float alpha = std::clamp(std::abs(tone) * 0.5f, 0.01f, 0.9f);
        toneFilterState_ += alpha * (output - toneFilterState_);
        const float highBoost = output + (output - toneFilterState_) * (tone * 1.5f);
        output = tone < 0.0f ? toneFilterState_ : highBoost;
    }

    const double dcPole = std::exp(-2.0 * std::numbers::pi * 25.0 / std::max(1.0, sampleRate));
    const double outD = static_cast<double>(output);
    const double blocked = outD - dcX_ + dcPole * dcY_;
    dcX_ = outD;
    dcY_ = blocked;
    return static_cast<float>(blocked);
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

    // Peak and RMS blended sidechain detector (80% peak, 20% RMS) for transparent musical snap
    const float peak = std::max(std::abs(left), std::abs(right));
    const float rms = std::sqrt(0.5f * (left * left + right * right));
    const float detectorSignal = peak * 0.82f + rms * 0.18f;
    const float inputDb = linToDb(detectorSignal);

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

void ThreeBandEq::prepare(double sampleRate) noexcept {
    sampleRate_ = std::max(8000.0, sampleRate);
    glideCoeff_ = 1.0f - std::exp(-1.0f / (0.020f * static_cast<float>(sampleRate_)));
    reset();
}

void ThreeBandEq::reset() noexcept {
    std::ranges::for_each(bands_, [](Band& band) {
        band.z1L = band.z2L = band.z1R = band.z2R = 0.0;
        band.b0 = 1.0;
        band.b1 = band.b2 = band.a1 = band.a2 = 0.0;
        band.initialized = false;
    });
}

void ThreeBandEq::updateBand(Band& band, BandType type, float minHz, float maxHz,
                             float targetHz, float targetDb) noexcept {
    const float clampedHz = std::clamp(std::isfinite(targetHz) ? targetHz : minHz,
                                       minHz, std::min(maxHz, static_cast<float>(sampleRate_ * 0.45)));
    const float targetLogFreq = std::log(clampedHz);
    const float clampedDb = std::clamp(std::isfinite(targetDb) ? targetDb : 0.0f, -12.0f, 12.0f);
    if (!band.initialized) {
        band.smoothedLogFreq = targetLogFreq;
        band.smoothedDb = clampedDb;
        band.initialized = true;
    }
    band.smoothedLogFreq += glideCoeff_ * (targetLogFreq - band.smoothedLogFreq);
    band.smoothedDb += glideCoeff_ * (clampedDb - band.smoothedDb);

    if (std::abs(band.smoothedDb) < 0.005f) {
        band.b0 = 1.0;
        band.b1 = band.b2 = band.a1 = band.a2 = 0.0;
        return;
    }

    const double freq = static_cast<double>(std::exp(band.smoothedLogFreq));
    const double omega = 2.0 * std::numbers::pi * std::clamp(freq / sampleRate_, 1.0e-5, 0.45);
    const double sn = std::sin(omega);
    const double cs = std::cos(omega);
    const double amp = std::pow(10.0, static_cast<double>(band.smoothedDb) / 40.0);

    if (type == BandType::peak) {
        const double alpha = sn / (2.0 * 0.9);
        const double a0 = 1.0 + alpha / amp;
        band.b0 = (1.0 + alpha * amp) / a0;
        band.b1 = (-2.0 * cs) / a0;
        band.b2 = (1.0 - alpha * amp) / a0;
        band.a1 = (-2.0 * cs) / a0;
        band.a2 = (1.0 - alpha / amp) / a0;
        return;
    }

    const double alpha = sn * 0.5 * std::sqrt(2.0);
    const double twoRootAmpAlpha = 2.0 * std::sqrt(amp) * alpha;
    if (type == BandType::lowShelf) {
        const double a0 = (amp + 1.0) + (amp - 1.0) * cs + twoRootAmpAlpha;
        band.b0 = (amp * ((amp + 1.0) - (amp - 1.0) * cs + twoRootAmpAlpha)) / a0;
        band.b1 = (2.0 * amp * ((amp - 1.0) - (amp + 1.0) * cs)) / a0;
        band.b2 = (amp * ((amp + 1.0) - (amp - 1.0) * cs - twoRootAmpAlpha)) / a0;
        band.a1 = (-2.0 * ((amp - 1.0) + (amp + 1.0) * cs)) / a0;
        band.a2 = ((amp + 1.0) + (amp - 1.0) * cs - twoRootAmpAlpha) / a0;
        return;
    }

    const double a0 = (amp + 1.0) - (amp - 1.0) * cs + twoRootAmpAlpha;
    band.b0 = (amp * ((amp + 1.0) + (amp - 1.0) * cs + twoRootAmpAlpha)) / a0;
    band.b1 = (-2.0 * amp * ((amp - 1.0) + (amp + 1.0) * cs)) / a0;
    band.b2 = (amp * ((amp + 1.0) + (amp - 1.0) * cs - twoRootAmpAlpha)) / a0;
    band.a1 = (2.0 * ((amp - 1.0) - (amp + 1.0) * cs)) / a0;
    band.a2 = ((amp + 1.0) - (amp - 1.0) * cs - twoRootAmpAlpha) / a0;
}

void ThreeBandEq::filterBand(Band& band, float& left, float& right) noexcept {
    const double xL = static_cast<double>(left);
    const double yL = band.b0 * xL + band.z1L;
    band.z1L = band.b1 * xL - band.a1 * yL + band.z2L;
    band.z2L = band.b2 * xL - band.a2 * yL;
    left = std::isfinite(yL) ? static_cast<float>(yL) : 0.0f;

    const double xR = static_cast<double>(right);
    const double yR = band.b0 * xR + band.z1R;
    band.z1R = band.b1 * xR - band.a1 * yR + band.z2R;
    band.z2R = band.b2 * xR - band.a2 * yR;
    right = std::isfinite(yR) ? static_cast<float>(yR) : 0.0f;
}

void ThreeBandEq::process(float& left, float& right, const ThreeBandEqSettings& settings) noexcept {
    updateBand(bands_[0], BandType::lowShelf, 25.0f, 400.0f, settings.lowFreqHz, settings.lowGainDb);
    updateBand(bands_[1], BandType::peak, 200.0f, 6000.0f, settings.midFreqHz, settings.midGainDb);
    updateBand(bands_[2], BandType::highShelf, 2000.0f, 18000.0f, settings.highFreqHz, settings.highGainDb);
    filterBand(bands_[0], left, right);
    filterBand(bands_[1], left, right);
    filterBand(bands_[2], left, right);
}

// ============================================================================
// BrickwallLimiter
// ============================================================================

void BrickwallLimiter::prepare(double sampleRate) {
    sampleRate_ = std::max(1.0, sampleRate);
    reset();
}

void BrickwallLimiter::reset() noexcept {
    delayL_.fill(0.0f);
    delayR_.fill(0.0f);
    tpHistL_.fill(0.0f);
    tpHistR_.fill(0.0f);
    scDelay_.fill(1.0f);
    minWindow_.fill(1.0f);
    smoothBuf_.fill(1.0f);
    writePos_ = 0;
    tpPos_ = 0;
    scPos_ = 0;
    minPos_ = 0;
    smoothPos_ = 0;
    envGain_ = 1.0f;
}

void BrickwallLimiter::process(float& left, float& right, const LimiterSettings& settings) noexcept {
    const float delayedL = delayL_[writePos_];
    const float delayedR = delayR_[writePos_];
    delayL_[writePos_] = left;
    delayR_[writePos_] = right;
    writePos_ = (writePos_ + 1) % delayL_.size();

    tpPos_ = (tpPos_ + 7) & 7u;
    tpHistL_[tpPos_] = left;
    tpHistR_[tpPos_] = right;

    // 4x polyphase Hamming-windowed sinc true-peak detector catches inter-sample overshoots before D/A reconstruction.
    constexpr std::array<std::array<float, 8>, 3> kTruePeakPhases{{
        { -0.01094f, 0.04517f, -0.14368f, 0.89501f, 0.27766f, -0.08123f, 0.02335f, -0.00534f },
        { -0.01054f, 0.04647f, -0.15245f, 0.61652f, 0.61652f, -0.15245f, 0.04647f, -0.01054f },
        { -0.00534f, 0.02335f, -0.08123f, 0.27766f, 0.89501f, -0.14368f, 0.04517f, -0.01094f }
    }};
    float truePeak = std::max(std::abs(left), std::abs(right));
    std::ranges::for_each(kTruePeakPhases, [&](const auto& coeffs) {
        float accL = 0.0f;
        float accR = 0.0f;
        std::ranges::for_each(std::views::iota(size_t{ 0 }, size_t{ 8 }), [&](size_t m) {
            const size_t idx = (tpPos_ + m) & 7u;
            accL += coeffs[m] * tpHistL_[idx];
            accR += coeffs[m] * tpHistR_[idx];
        });
        truePeak = std::max({ truePeak, std::abs(accL), std::abs(accR) });
    });

    const float ceiling = dbToLin(settings.ceilingDb);
    const float desiredGain = (settings.enabled && truePeak > ceiling && truePeak > 1.0e-6f)
        ? (ceiling / truePeak)
        : 1.0f;

    // 18-sample pre-delay + 49-sample hold + 40-sample smoother aligns the gain trough at sample 64.
    const float scTarget = scDelay_[scPos_];
    scDelay_[scPos_] = desiredGain;
    scPos_ = (scPos_ + 1) % scDelay_.size();

    minWindow_[minPos_] = scTarget;
    minPos_ = (minPos_ + 1) % minWindow_.size();
    const float holdTarget = *std::ranges::min_element(minWindow_);

    const float releaseCoeff = std::exp(-1.0f / (std::max(1.0f, settings.releaseMs) * 0.001f * static_cast<float>(sampleRate_)));
    const float released = holdTarget + releaseCoeff * (envGain_ - holdTarget);
    envGain_ = holdTarget < envGain_ ? holdTarget : released;

    smoothBuf_[smoothPos_] = envGain_;
    smoothPos_ = (smoothPos_ + 1) % smoothBuf_.size();

    if (!settings.enabled) {
        left = delayedL;
        right = delayedR;
        return;
    }

    double sumGain = 0.0;
    std::ranges::for_each(smoothBuf_, [&](float g) { sumGain += static_cast<double>(g); });
    const float smoothGain = static_cast<float>(sumGain * (1.0 / static_cast<double>(smoothBuf_.size())));

    left = std::clamp(delayedL * smoothGain, -ceiling, ceiling);
    right = std::clamp(delayedR * smoothGain, -ceiling, ceiling);
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
