#include "ClapSynth.h"
#include <algorithm>
#include <cmath>
#include <numbers>
#include <ranges>

namespace openclapper::core {

namespace {

[[nodiscard]] constexpr float pi() noexcept {
    return std::numbers::pi_v<float>;
}

[[nodiscard]] float computeNoiseTailEnvelope(float mainAge,
                                             bool ampEnabled,
                                             float attackSamples,
                                             float holdSamples,
                                             float decaySamples,
                                             float curve) noexcept {
    if (mainAge < 0.0f) {
        return 0.0f;
    }
    if (ampEnabled) {
        return 1.0f;
    }
    if (mainAge < attackSamples && attackSamples > 0.0f) {
        return mainAge / attackSamples;
    }
    if (mainAge < attackSamples + holdSamples) {
        return 1.0f;
    }
    const float decayT = mainAge - attackSamples - holdSamples;
    if (decayT >= decaySamples) {
        return 0.0f;
    }
    // Curve only changes how fast the exponential falls. The tail always
    // settles to silence instead of holding a shelf and then stopping.
    const float clampedCurve = std::clamp(curve, -1.0f, 1.0f);
    const float tauScale = 0.20f + (clampedCurve + 1.0f) * 0.5f * 0.42f;
    const float tau = std::max(1.0f, decaySamples * tauScale);
    const float progress = decayT / decaySamples;
    float window = 1.0f;
    if (progress > 0.82f) {
        const float fade = (progress - 0.82f) / 0.18f;
        window = 0.5f * (1.0f + std::cos(pi() * fade));
    }
    return std::exp(-decayT / tau) * window;
}

// Fast xorshift32 PRNG
[[nodiscard]] inline float nextRandomFloat(uint32_t& state) noexcept {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return (static_cast<float>(state) * (2.0f / 4294967296.0f)) - 1.0f;
}

[[nodiscard]] inline double nextUniform01(uint32_t& state) noexcept {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return static_cast<double>(state >> 8) * (1.0 / 16777216.0);
}

// Biquad highpass filter coefficients (12 dB/oct)
void computeHighpassCoeffs(float freqHz, double sampleRate, float& b0, float& b1, float& b2, float& a1, float& a2) noexcept {
    const float nyquist = static_cast<float>(sampleRate) * 0.5f;
    const float clampedFreq = std::clamp(freqHz, 10.0f, nyquist * 0.95f);
    const float w0 = 2.0f * pi() * clampedFreq / static_cast<float>(sampleRate);
    const float cosW0 = std::cos(w0);
    const float alpha = std::sin(w0) * 0.70710678f; // Q = 0.707 (Butterworth)

    const float a0 = 1.0f + alpha;
    b0 = ((1.0f + cosW0) * 0.5f) / a0;
    b1 = (-(1.0f + cosW0)) / a0;
    b2 = ((1.0f + cosW0) * 0.5f) / a0;
    a1 = (-2.0f * cosW0) / a0;
    a2 = (1.0f - alpha) / a0;
}

// Biquad lowpass filter coefficients (12 dB/oct)
void computeLowpassCoeffs(float freqHz, double sampleRate, float& b0, float& b1, float& b2, float& a1, float& a2) noexcept {
    const float nyquist = static_cast<float>(sampleRate) * 0.5f;
    const float clampedFreq = std::clamp(freqHz, 20.0f, nyquist * 0.95f);
    const float w0 = 2.0f * pi() * clampedFreq / static_cast<float>(sampleRate);
    const float cosW0 = std::cos(w0);
    const float alpha = std::sin(w0) * 0.70710678f; // Q = 0.707 (Butterworth)

    const float a0 = 1.0f + alpha;
    b0 = ((1.0f - cosW0) * 0.5f) / a0;
    b1 = (1.0f - cosW0) / a0;
    b2 = ((1.0f - cosW0) * 0.5f) / a0;
    a1 = (-2.0f * cosW0) / a0;
    a2 = (1.0f - alpha) / a0;
}

void computeBandpassCoeffs(float freqHz, float q, double sampleRate, float& b0, float& b1, float& b2, float& a1, float& a2) noexcept {
    const float nyquist = static_cast<float>(sampleRate) * 0.5f;
    const float clampedFreq = std::clamp(freqHz, 20.0f, nyquist * 0.95f);
    const float clampedQ = std::clamp(q, 0.1f, 30.0f);
    const float w0 = 2.0f * pi() * clampedFreq / static_cast<float>(sampleRate);
    const float alpha = std::sin(w0) / (2.0f * clampedQ);
    const float cosW0 = std::cos(w0);
    const float a0 = 1.0f + alpha;
    b0 = alpha / a0;
    b1 = 0.0f;
    b2 = -alpha / a0;
    a1 = (-2.0f * cosW0) / a0;
    a2 = (1.0f - alpha) / a0;
}

// Process one biquad sample Direct Form II transposed or Direct Form I
[[nodiscard]] inline float processBiquad(float in, BiquadState& s, float b0, float b1, float b2, float a1, float a2) noexcept {
    const float out = b0 * in + b1 * s.x1 + b2 * s.x2 - a1 * s.y1 - a2 * s.y2;
    s.x2 = s.x1;
    s.x1 = in;
    s.y2 = s.y1;
    s.y1 = out;
    return out;
}

} // namespace

StereoFrame ImpactSaturator::process(float inL, float inR, float amount, float driveScale, double sampleRate) noexcept {
    const float clampedAmount = std::clamp(amount, 0.0f, 1.0f);
    const float peak = std::max({ std::abs(inL), std::abs(inR), 1.0e-5f });
    const float detDb = 20.0f * std::log10(peak);
    const float coeff2ms = 1.0f - std::exp(-1.0f / std::max(1.0f, 0.002f * static_cast<float>(sampleRate)));
    envDb += coeff2ms * (detDb - envDb);

    const float overDb = std::max(0.0f, envDb - (-11.5f));
    const float compDb = (5.8f - overDb * 0.50f) * clampedAmount;
    const float compGain = std::exp2(compDb * (1.0f / 6.0205999f));
    const double drive = 0.03 + static_cast<double>(driveScale * clampedAmount);
    const double invTanhDrive = 1.0 / std::tanh(drive);

    const auto shapeChannel = [&](double x, ChannelOversampler& ch) {
        const auto [up0, up1] = ch.halfband.upsample(x);
        const double y0 = adaaTanh1(up0 * drive, ch.prevU, ch.prevF1) * invTanhDrive;
        const double y1 = adaaTanh1(up1 * drive, ch.prevU, ch.prevF1) * invTanhDrive;
        return static_cast<float>(ch.halfband.downsample(y0, y1));
    };

    return {
        shapeChannel(static_cast<double>(inL * compGain), leftChan),
        shapeChannel(static_cast<double>(inR * compGain), rightChan)
    };
}

void CoupledPalmResonator::configure(float freqHz, float q, double sampleRate) noexcept {
    const float sr = static_cast<float>(sampleRate);
    const float nyquistGuard = sr * 0.48f;
    const auto setupMode = [&](float modeHz, float modeQ, float& twoRCos, float& rSq, float& norm) {
        const float clampedHz = std::clamp(modeHz, 20.0f, nyquistGuard);
        const float w = 2.0f * pi() * clampedHz / sr;
        const float cosW = std::cos(w);
        const float r = std::clamp(std::exp(-pi() * clampedHz / (std::max(0.4f, modeQ) * sr)), 0.35f, 0.996f);
        const float r2 = r * r;
        const float denom = std::max(1.0e-6f, (1.0f + r2) * (1.0f + r2) - 4.0f * r2 * cosW * cosW);
        twoRCos = 2.0f * r * cosW;
        rSq = r2;
        norm = 0.54f * std::sqrt(((1.0f - r2) * denom) / (1.0f + r2));
        return r;
    };
    const float rPrimary = setupMode(freqHz, q, twoRCosA, rSqA, normA);
    (void)setupMode(freqHz * 1.39f, std::min(2.4f, q * 0.45f), twoRCosB, rSqB, normB);
    coupling = 0.04f * (1.0f - rPrimary);
}

// ============================================================================
// GatedStereoReverb
// ============================================================================

void GatedStereoReverb::prepare(double sampleRate) {
    const size_t apMargin = static_cast<size_t>(std::max(8.0, sampleRate * 0.001));
    diffL1_.prepare(std::max<size_t>(8, static_cast<size_t>(sampleRate * 0.0031)), 0.62f, apMargin);
    diffL2_.prepare(std::max<size_t>(8, static_cast<size_t>(sampleRate * 0.0079)), 0.58f, apMargin);
    diffL3_.prepare(std::max<size_t>(8, static_cast<size_t>(sampleRate * 0.0113)), 0.54f, apMargin);
    diffR1_.prepare(std::max<size_t>(8, static_cast<size_t>(sampleRate * 0.0023)), 0.62f, apMargin);
    diffR2_.prepare(std::max<size_t>(8, static_cast<size_t>(sampleRate * 0.0058)), 0.58f, apMargin);
    diffR3_.prepare(std::max<size_t>(8, static_cast<size_t>(sampleRate * 0.0097)), 0.54f, apMargin);

    const int margin = 8;
    delayL1_.assign(std::max(16, static_cast<int>(sampleRate * 0.0297)) + margin, 0.0f);
    delayL2_.assign(std::max(16, static_cast<int>(sampleRate * 0.0371)) + margin, 0.0f);
    delayL3_.assign(std::max(16, static_cast<int>(sampleRate * 0.0439)) + margin, 0.0f);
    delayR1_.assign(std::max(16, static_cast<int>(sampleRate * 0.0313)) + margin, 0.0f);
    delayR2_.assign(std::max(16, static_cast<int>(sampleRate * 0.0419)) + margin, 0.0f);
    delayR3_.assign(std::max(16, static_cast<int>(sampleRate * 0.0487)) + margin, 0.0f);
    reset();
}

void GatedStereoReverb::reset() noexcept {
    diffL1_.reset();
    diffL2_.reset();
    diffL3_.reset();
    diffR1_.reset();
    diffR2_.reset();
    diffR3_.reset();
    std::fill(delayL1_.begin(), delayL1_.end(), 0.0f);
    std::fill(delayL2_.begin(), delayL2_.end(), 0.0f);
    std::fill(delayL3_.begin(), delayL3_.end(), 0.0f);
    std::fill(delayR1_.begin(), delayR1_.end(), 0.0f);
    std::fill(delayR2_.begin(), delayR2_.end(), 0.0f);
    std::fill(delayR3_.begin(), delayR3_.end(), 0.0f);
    writeL1_ = writeL2_ = writeL3_ = 0;
    writeR1_ = writeR2_ = writeR3_ = 0;
    dampStates_.fill(0.0f);
    hpfStateL_ = hpfStateR_ = 0.0f;
    lfoPhaseL_ = 0.0;
    lfoPhaseR_ = 0.25;
    sampleCounter_ = 0;
    gateEnvelope_ = 1.0f;
}

void GatedStereoReverb::triggerGate() noexcept {
    sampleCounter_ = 0;
    gateEnvelope_ = 1.0f;
}

StereoFrame GatedStereoReverb::process(
    float inLeft,
    float inRight,
    float size,
    float decayMs,
    float damp,
    float gateMs,
    double sampleRate) noexcept {
    if (delayL1_.empty()) {
        return { inLeft, inRight };
    }

    sampleCounter_++;

    gateEnvelope_ = 1.0f;
    if (gateMs > 0.0f) {
        const float gateSamples = (gateMs * 0.001f) * static_cast<float>(sampleRate);
        const float releaseSamples = std::max(1.0f, (kReverbGateReleaseMs * 0.001f) * static_cast<float>(sampleRate));
        const float elapsed = static_cast<float>(sampleCounter_);
        if (elapsed >= gateSamples + releaseSamples) {
            gateEnvelope_ = 0.0f;
        }
        if (elapsed > gateSamples && elapsed < gateSamples + releaseSamples) {
            const float fade = (elapsed - gateSamples) / releaseSamples;
            gateEnvelope_ = 0.5f * (1.0f + std::cos(pi() * fade));
        }
    }

    // Input highpass filter (140 Hz) to keep the reverb tank clean from low rumble
    const float hpfCoeff = static_cast<float>(1.0 - std::exp(-2.0 * pi() * 140.0 / sampleRate));
    hpfStateL_ += hpfCoeff * (inLeft - hpfStateL_);
    const float cleanInL = inLeft - hpfStateL_;
    hpfStateR_ += hpfCoeff * (inRight - hpfStateR_);
    const float cleanInR = inRight - hpfStateR_;

    // Modulate strictly inside serial allpass diffusers with zero-sum excursion so comb pitch never wobbles
    lfoPhaseL_ += 2.0 * pi() * 0.73 / sampleRate;
    if (lfoPhaseL_ > 2.0 * pi()) lfoPhaseL_ -= 2.0 * pi();
    lfoPhaseR_ += 2.0 * pi() * 0.91 / sampleRate;
    if (lfoPhaseR_ > 2.0 * pi()) lfoPhaseR_ -= 2.0 * pi();

    const double modDepth = 0.000018 * sampleRate;
    const double modL = std::sin(lfoPhaseL_) * modDepth;
    const double modR = std::cos(lfoPhaseR_) * modDepth;

    const float diffInL = diffL3_.processModulated(
        diffL2_.processModulated(diffL1_.processModulated(cleanInL, modL), -0.7 * modL), -0.3 * modL);
    const float diffInR = diffR3_.processModulated(
        diffR2_.processModulated(diffR1_.processModulated(cleanInR, modR), -0.7 * modR), -0.3 * modR);

    const double sizeScale = 0.55 + 0.45 * std::clamp(static_cast<double>(size), 0.0, 1.0);
    constexpr std::array<double, 6> kBaseTimesSec{ 0.0297, 0.0371, 0.0439, 0.0313, 0.0419, 0.0487 };
    const double t60Sec = std::max(0.01, static_cast<double>(decayMs) * 0.001);
    const float dampFactor = std::clamp(damp, 0.0f, 0.85f);

    const std::array<float, 6> rawOuts{
        readHermiteCircular(delayL1_, kBaseTimesSec[0] * sizeScale * sampleRate, static_cast<size_t>(writeL1_)),
        readHermiteCircular(delayL2_, kBaseTimesSec[1] * sizeScale * sampleRate, static_cast<size_t>(writeL2_)),
        readHermiteCircular(delayL3_, kBaseTimesSec[2] * sizeScale * sampleRate, static_cast<size_t>(writeL3_)),
        readHermiteCircular(delayR1_, kBaseTimesSec[3] * sizeScale * sampleRate, static_cast<size_t>(writeR1_)),
        readHermiteCircular(delayR2_, kBaseTimesSec[4] * sizeScale * sampleRate, static_cast<size_t>(writeR2_)),
        readHermiteCircular(delayR3_, kBaseTimesSec[5] * sizeScale * sampleRate, static_cast<size_t>(writeR3_))
    };

    // Exact per-line RT60 feedback g_i = 10^(-3 * tau_i / T60) and independent per-loop HF damping
    std::array<float, 6> dampedScaled{};
    float householderSum = 0.0f;
    std::ranges::for_each(std::views::iota(size_t{ 0 }, size_t{ 6 }), [&](size_t i) {
        const double tauSec = kBaseTimesSec[i] * sizeScale;
        const float gi = static_cast<float>(std::clamp(std::pow(10.0, -3.0 * tauSec / t60Sec), 0.0001, 0.995));
        dampStates_[i] = dampStates_[i] * dampFactor + rawOuts[i] * (1.0f - dampFactor);
        dampedScaled[i] = dampStates_[i] * gi;
        householderSum += dampedScaled[i];
    });
    const float commonMix = householderSum * (1.0f / 3.0f);

    // Unitary 6x6 Householder feedback matrix preserves homogeneous RT60 across all delay lengths
    delayL1_[static_cast<size_t>(writeL1_)] = diffInL * 0.55f + (dampedScaled[0] - commonMix);
    delayL2_[static_cast<size_t>(writeL2_)] = -diffInL * 0.42f + (dampedScaled[1] - commonMix);
    delayL3_[static_cast<size_t>(writeL3_)] = diffInL * 0.34f + (dampedScaled[2] - commonMix);
    delayR1_[static_cast<size_t>(writeR1_)] = diffInR * 0.55f + (dampedScaled[3] - commonMix);
    delayR2_[static_cast<size_t>(writeR2_)] = diffInR * 0.42f + (dampedScaled[4] - commonMix);
    delayR3_[static_cast<size_t>(writeR3_)] = -diffInR * 0.34f + (dampedScaled[5] - commonMix);

    writeL1_ = (writeL1_ + 1) % static_cast<int>(delayL1_.size());
    writeL2_ = (writeL2_ + 1) % static_cast<int>(delayL2_.size());
    writeL3_ = (writeL3_ + 1) % static_cast<int>(delayL3_.size());
    writeR1_ = (writeR1_ + 1) % static_cast<int>(delayR1_.size());
    writeR2_ = (writeR2_ + 1) % static_cast<int>(delayR2_.size());
    writeR3_ = (writeR3_ + 1) % static_cast<int>(delayR3_.size());

    const float wetL = 0.56f * (rawOuts[0] - rawOuts[1] + rawOuts[2]);
    const float wetR = 0.56f * (rawOuts[3] + rawOuts[4] - rawOuts[5]);
    const float outLeft = (wetL - wetR * 0.22f) * gateEnvelope_;
    const float outRight = (wetR - wetL * 0.22f) * gateEnvelope_;
    return { outLeft, outRight };
}

// ============================================================================
// ClapSynth
// ============================================================================

void ClapSynth::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
    constexpr std::array poles{0.99765, 0.96300, 0.57000};
    constexpr std::array gains{0.0990460, 0.2965164, 1.0526913};
    for (size_t i = 0; i < poles.size(); ++i) {
        const auto pole = std::pow(poles[i], 44100.0 / sampleRate);
        pinkPoles_[i] = static_cast<float>(pole);
        pinkGains_[i] = static_cast<float>(gains[i] * (1.0 - pole) / (1.0 - poles[i]));
    }
    lofiStep_ = std::min(1.0, 12000.0 / sampleRate);
    velvetTd_ = std::max(1.5, sampleRate / 16000.0);
    velvetGain_ = static_cast<float>(std::sqrt(0.39 * velvetTd_));
    contactRiseSteps_ = static_cast<float>(1.0 / std::max(1.0, 0.00035 * sampleRate));
    reset();
}

void ClapSynth::reset() noexcept {
    isPlaying_ = false;
    amplitude_ = 0.0f;
    releaseAge_ = -1;
    preHPF_L_.reset();
    preHPF_R_.reset();
    preLPF_L_.reset();
    preLPF_R_.reset();
    preHPF2_L_.reset();
    preHPF2_R_.reset();
    preLPF2_L_.reset();
    preLPF2_R_.reset();
    totalSamplesPlayed_ = 0;
    totalDurationSamples_ = 0;
    mainStartSamples_ = 0;
    burstCount_ = 0;
    std::ranges::for_each(burstFormantStates_, [](BiquadState& s) { s.reset(); });
    palmModalL_.reset();
    palmModalR_.reset();
    toneBloomFilterL_.reset();
    toneBloomFilterR_.reset();
    clickPhase_ = 0.0;
    clickPrevWavelet_ = 0.0f;
    impactNoiseSat_.reset();
    impactToneSat_.reset();
    impactLpL_ = 0.0f;
    impactLpR_ = 0.0f;
    toneHPF_L_.reset();
    toneHPF_R_.reset();
    toneLPF_L_.reset();
    toneLPF_R_.reset();
    lastToneFrequency_ = -1.0f;
    lastToneQ_ = -1.0f;
    lastToneHPF_ = -1.0f;
    lastToneLPF_ = -1.0f;
    lastHPF_ = -1.0f;
    lastLPF_ = -1.0f;
    noiseHPF_L_.reset();
    noiseHPF_R_.reset();
    noiseLPF_L_.reset();
    noiseLPF_R_.reset();
    noiseHPF2_L_.reset();
    noiseHPF2_R_.reset();
    noiseLPF2_L_.reset();
    noiseLPF2_R_.reset();
    pinkLeft_.fill(0.0f);
    pinkRight_.fill(0.0f);
    velvetL_ = { 0, 0, 0, 0x243f6a88U };
    velvetR_ = { 0, 0, 0, 0x85a308d3U };
    lofiHeld_ = {};
    lofiPhase_ = 1.0;
}

void ClapSynth::trigger(const ClapSynthSettings& settings, int midiNote, float velocity) noexcept {
    reset();
    isPlaying_ = true;
    ampEnabled_ = settings.ampEnabled;
    velocity_ = std::clamp(velocity, 0.0f, 1.0f);
    midiNote_ = std::clamp(midiNote, 0, 127);
    ++triggerCount_;
    const auto spreadQuant = static_cast<uint32_t>(std::lround(std::clamp(settings.preClapSpreadMs, 0.0f, kMaxTimingOffsetMs) * 100.0f));
    const auto decayQuant = static_cast<uint32_t>(std::lround(std::clamp(settings.noiseDecayMs, 1.0f, 5000.0f) * 10.0f));
    const uint32_t timingSalt = (spreadQuant ^ (decayQuant * 0x27d4eb2dU)) * 0x165667b1U;
    prngState_ = 0x9e3779b9U ^ (static_cast<uint32_t>(midiNote_) * 0x85ebca6bU) ^ (triggerCount_ * 0xc2b2ae35U) ^ timingSalt;
    if (prngState_ == 0) {
        prngState_ = 0x12345678U;
    }

    burstCount_ = std::clamp(settings.preClapCount, 0, 5);
    // The main strike is the note-on. Any flam is squeezed into the first 5 ms so it
    // reads as one tight clap, not a late hit.
    mainStartSamples_ = 0;
    const float spreadSamples = flamSpacingMs(burstCount_, settings.preClapSpreadMs) * 0.001f * static_cast<float>(sampleRate_);
    const int maxOffsetSamples = std::max(0, static_cast<int>(std::lround(kMaxTimingOffsetMs * 0.001 * sampleRate_)));

    const float baseFormantHz = std::clamp(1350.0f + midiNoteToHz(midiNote_) * 0.35f, 1000.0f, 2400.0f);
    std::ranges::for_each(std::views::iota(0, burstCount_), [&](int i) {
        const auto idx = static_cast<size_t>(i);
        const float jitter = std::clamp(settings.preClapJitter, 0.0f, 1.0f);
        const float jitterTime = nextRandomFloat(prngState_) * 0.5f * jitter * spreadSamples;
        const int start = static_cast<int>(std::lround(static_cast<float>(i) * spreadSamples + jitterTime));
        burstStartSamples_[idx] = std::clamp(start, 0, maxOffsetSamples);
        const float jitterAmp = 1.0f + nextRandomFloat(prngState_) * 0.2f * jitter;
        const float progression = burstCount_ > 1 ? static_cast<float>(i) / static_cast<float>(burstCount_ - 1) : 1.0f;
        const float pressure = 0.63f + 0.37f * progression;
        burstAmplitudes_[idx] = std::clamp(settings.preClapLevel * pressure * jitterAmp, 0.0f, 1.0f);
        const float dir = (i % 2 == 0) ? -1.0f : 1.0f;
        const float spreadFrac = static_cast<float>(i + 1) / static_cast<float>(std::max(1, burstCount_));
        burstPans_[idx] = std::clamp(dir * settings.preClapFlamSpread * spreadFrac, -1.0f, 1.0f);

        const uint32_t handSeed = (prngState_ ^ (static_cast<uint32_t>(i + 1) * 0x9e3779b9U)) * 0x85ebca6bU;
        burstPrngStates_[idx] = handSeed != 0U ? handSeed : 0x6d2b79f5U;
        const float formantVariation = nextRandomFloat(burstPrngStates_[idx]) * 0.08f;
        const float handFormantHz = baseFormantHz * (1.0f + formantVariation);
        auto& coeffs = burstFormantCoeffs_[idx];
        computeBandpassCoeffs(handFormantHz, 0.58f, sampleRate_, coeffs.b0, coeffs.b1, coeffs.b2, coeffs.a1, coeffs.a2);
    });

    const float pitchDetuneSemitones = (settings.pitchHumanizeCents > 0.0f)
        ? nextRandomFloat(prngState_) * (std::clamp(settings.pitchHumanizeCents, 0.0f, 100.0f) / 100.0f)
        : 0.0f;
    pitchHumanizeRatio_ = std::exp2(pitchDetuneSemitones / 12.0f);

    const uint32_t vSeedL = (prngState_ ^ 0x243f6a88U) * 0x9e3779b1U;
    const uint32_t vSeedR = (prngState_ ^ 0x85a308d3U) * 0xc2b2ae3dU;
    velvetL_.prng = vSeedL != 0U ? vSeedL : 0x243f6a88U;
    velvetR_.prng = vSeedR != 0U ? vSeedR : 0x85a308d3U;
    velvetL_.sampleIndex = 0;
    velvetR_.sampleIndex = 0;
    velvetL_.gridIndex = 0;
    velvetR_.gridIndex = 0;
    velvetL_.nextPulseSample = static_cast<int64_t>(std::floor(nextUniform01(velvetL_.prng) * velvetTd_));
    velvetR_.nextPulseSample = static_cast<int64_t>(std::floor(nextUniform01(velvetR_.prng) * velvetTd_));

    const float mainStartMs = 0.0f;
    const float burstTailMs = burstCount_ > 0
        ? kMaxTimingOffsetMs + settings.preClapDecayMs * 4.0f
        : 0.0f;
    const float noiseTailMs = settings.noiseAttackMs + settings.noiseHoldMs + settings.noiseDecayMs * 4.0f;
    const float clickScale = settings.clickType == ClickType::Thump ? 2.5f : 1.0f;
    const float clickTailMs = std::clamp(settings.clickDecayMs, 0.2f, 20.0f) * clickScale * 6.0f;
    const float mainTailMs = mainStartMs + std::max({ noiseTailMs, settings.toneDecayMs * 6.0f, clickTailMs, 20.0f });
    const float totalTimeMs = std::max(burstTailMs, mainTailMs);
    totalDurationSamples_ = static_cast<int>(totalTimeMs * 0.001f * static_cast<float>(sampleRate_));
}

StereoFrame ClapSynth::generateNoise(NoiseType type) noexcept {
    const StereoFrame white{nextRandomFloat(prngState_), nextRandomFloat(prngState_)};
    if (type == NoiseType::Pink) {
        const auto filter = [&](float input, std::array<float, 3>& memory) {
            float sum = input * 0.1848f;
            for (size_t i = 0; i < memory.size(); ++i) {
                memory[i] = pinkPoles_[i] * memory[i] + input * pinkGains_[i];
                sum += memory[i];
            }
            return sum * 0.35f;
        };
        return {filter(white.left, pinkLeft_), filter(white.right, pinkRight_)};
    }
    if (type == NoiseType::Velvet) {
        const auto stepVelvet = [&](VelvetChannelState& state) {
            float sample = 0.0f;
            while (state.sampleIndex >= state.nextPulseSample) {
                if (state.sampleIndex == state.nextPulseSample) {
                    const float randPulse = nextRandomFloat(state.prng);
                    const float sign = randPulse >= 0.0f ? 1.0f : -1.0f;
                    sample = sign * velvetGain_;
                }
                ++state.gridIndex;
                const double jitter = nextUniform01(state.prng);
                state.nextPulseSample = static_cast<int64_t>(std::floor((static_cast<double>(state.gridIndex) + jitter) * velvetTd_));
            }
            ++state.sampleIndex;
            return sample;
        };
        return {stepVelvet(velvetL_), stepVelvet(velvetR_)};
    }
    if (type == NoiseType::LoFi) {
        if (lofiPhase_ >= 1.0) {
            lofiPhase_ -= 1.0;
            lofiHeld_ = {std::round(white.left * 15.0f) / 15.0f, std::round(white.right * 15.0f) / 15.0f};
        }
        lofiPhase_ += lofiStep_;
        return lofiHeld_;
    }
    return white;
}

float ClapSynth::getEnvelopeProgress() const noexcept {
    if (!isPlaying_ || totalDurationSamples_ <= 0) {
        return 1.0f;
    }
    return std::clamp(static_cast<float>(totalSamplesPlayed_) / static_cast<float>(totalDurationSamples_), 0.0f, 1.0f);
}

void ClapSynth::noteOff(const ClapSynthSettings& settings) noexcept {
    if (!ampEnabled_ || releaseAge_ >= 0) return;
    releaseStart_ = amplitude_;
    releaseSamples_ = static_cast<int>(std::clamp(settings.ampReleaseMs, 0.0f, 20000.0f) * 0.001 * sampleRate_);
    releaseAge_ = 0;
}

StereoFrame ClapSynth::processSample(const ClapSynthSettings& settings, bool* mainHitStarted,
                                     std::array<StereoFrame, 4>* components) noexcept {
    if (components != nullptr) *components = {};

    if (mainHitStarted != nullptr) {
        *mainHitStarted = false;
    }

    if (!isPlaying_) {
        return { 0.0f, 0.0f };
    }

    const auto currentSample = totalSamplesPlayed_++;
    if (mainHitStarted != nullptr && currentSample == mainStartSamples_) {
        *mainHitStarted = true;
    }
    if (!ampEnabled_ && currentSample >= totalDurationSamples_) {
        isPlaying_ = false;
        return { 0.0f, 0.0f };
    }

    if (ampEnabled_) {
        const double attack = std::max(0.0f, settings.ampAttackMs) * 0.001 * sampleRate_;
        const double decay = std::max(0.0f, settings.ampDecayMs) * 0.001 * sampleRate_;
        const float sustain = std::clamp(settings.ampSustain, 0.0f, 1.0f);
        if (releaseAge_ >= 0) {
            if (releaseAge_ >= releaseSamples_) {
                isPlaying_ = false;
                return {};
            }
            amplitude_ = releaseStart_ * (1.0f - static_cast<float>(releaseAge_++) / static_cast<float>(releaseSamples_));
        } else if (currentSample < attack) {
            amplitude_ = static_cast<float>(currentSample / attack);
        } else if (currentSample < attack + decay) {
            amplitude_ = 1.0f - (1.0f - sustain) * static_cast<float>((currentSample - attack) / decay);
        } else {
            amplitude_ = sustain;
        }
    } else {
        amplitude_ = 1.0f;
    }

    std::array<float, 4> layerGains{};
    const bool anySolo = std::any_of(settings.layers.begin(), settings.layers.end(), [](const auto& layer) { return layer.solo; });
    for (size_t layer = 0; layer < layerGains.size(); ++layer) {
        const auto& control = settings.layers[layer];
        if (!control.mute && (!anySolo || control.solo))
            layerGains[layer] = amplitude_ * std::pow(velocity_, std::clamp(control.velocityResponse, 0.0f, 2.0f));
    }

    float burstSignalL = 0.0f;
    float burstSignalR = 0.0f;
    const float burstDecaySamples = std::max(1.0f, settings.preClapDecayMs * 0.001f * static_cast<float>(sampleRate_));
    std::ranges::for_each(std::views::iota(0, burstCount_), [&](int i) {
        const auto idx = static_cast<size_t>(i);
        const auto age = currentSample - burstStartSamples_[idx];
        if (age < 0 || static_cast<float>(age) >= burstDecaySamples * 4.0f) {
            return;
        }

        const float contact = std::min(1.0f, static_cast<float>(age) * contactRiseSteps_);
        const float env = contact * std::exp(-static_cast<float>(age) / burstDecaySamples);
        const float rawHand = nextRandomFloat(burstPrngStates_[idx]);
        const auto& fc = burstFormantCoeffs_[idx];
        const float formantHand = processBiquad(rawHand, burstFormantStates_[idx], fc.b0, fc.b1, fc.b2, fc.a1, fc.a2) * 1.85f;
        const float handSample = formantHand * env * burstAmplitudes_[idx];
        const float panAngle = (burstPans_[idx] + 1.0f) * (0.25f * pi());
        burstSignalL += handSample * std::cos(panAngle);
        burstSignalR += handSample * std::sin(panAngle);
    });

    const float mainAge = static_cast<float>(currentSample - mainStartSamples_);
    const float attackSamples = std::clamp(settings.noiseAttackMs, 0.0f, kMaxTimingOffsetMs)
        * 0.001f * static_cast<float>(sampleRate_);
    const float holdSamples = std::max(0.0f, settings.noiseHoldMs * 0.001f * static_cast<float>(sampleRate_));
    const float decaySamples = std::max(1.0f, settings.noiseDecayMs * 0.001f * static_cast<float>(sampleRate_));
    const float impact = std::clamp(settings.impactPunch, 0.0f, 1.0f);
    const float noiseEnv = computeNoiseTailEnvelope(
        mainAge, ampEnabled_, attackSamples, holdSamples, decaySamples, settings.noiseCurve);

    const float toneDecaySamples = std::max(1.0f, settings.toneDecayMs * 0.001f * static_cast<float>(sampleRate_));
    const float toneEnv = ampEnabled_ ? 1.0f : std::exp(-std::max(0.0f, mainAge) / toneDecaySamples);
    const auto [rawNoiseL, rawNoiseR] = generateNoise(settings.noiseType);

    float clickSample = 0.0f;
    const float sr = static_cast<float>(sampleRate_);
    const float clampedClickDecayMs = std::clamp(settings.clickDecayMs, 0.2f, 20.0f);
    if (mainAge >= 0.0f && settings.clickLevel > 0.001f) {
        const float nyquistClick = sr * 0.45f;
        float baseClickHz = std::clamp(settings.clickFreqHz, 200.0f, nyquistClick);
        float chirpRatio = 1.65f;
        float clickGain = 1.56f;
        float waveletMs = std::clamp(clampedClickDecayMs * 1.15f, 0.4f, 1.35f);
        if (settings.clickType == ClickType::Crack) {
            chirpRatio = 2.05f;
            clickGain = 1.85f;
            waveletMs = std::clamp(clampedClickDecayMs * 1.45f, 0.55f, 1.75f);
        }
        if (settings.clickType == ClickType::Thump) {
            baseClickHz = std::clamp(baseClickHz * 0.28f, 140.0f, std::min(480.0f, nyquistClick));
            chirpRatio = 2.20f;
            clickGain = 1.85f;
            waveletMs = clampedClickDecayMs * 2.5f;
        }
        const float extendedSupportMs = std::max(waveletMs, clampedClickDecayMs * 4.5f);
        const float supportMs = clampedClickDecayMs > 1.2f ? extendedSupportMs : waveletMs;
        const float supportSamples = std::max(4.0f, supportMs * 0.001f * sr);
        if (mainAge <= supportSamples + 1.0f) {
            float waveletPacket = 0.0f;
            if (mainAge < supportSamples) {
                const float u = mainAge / supportSamples;
                const float tauSamples = std::max(2.0f, (clampedClickDecayMs > 1.2f ? clampedClickDecayMs : waveletMs * 0.32f) * 0.001f * sr);
                const float chirpEnv = std::exp(-mainAge / std::max(1.5f, waveletMs * 0.35f * 0.001f * sr));
                const float instHz = std::clamp(baseClickHz * (1.0f + (chirpRatio - 1.0f) * chirpEnv), 40.0f, nyquistClick);
                const float wInst = 2.0f * pi() * instHz / sr;
                const float taper = std::cos(0.5f * pi() * u);
                const float minPhaseEnv = std::exp(-mainAge / tauSamples) * taper * taper;
                const float diffNorm = 1.0f / std::max(0.22f, 2.0f * std::sin(0.5f * wInst));
                float carrier = static_cast<float>(std::cos(clickPhase_));
                if (settings.clickType == ClickType::Crack && clampedClickDecayMs > 1.2f) {
                    const float tailBlend = std::clamp(mainAge / std::max(1.0f, waveletMs * 0.001f * sr), 0.0f, 1.0f);
                    carrier = carrier * (1.0f - 0.55f * tailBlend) + rawNoiseL * (0.65f * tailBlend);
                }
                waveletPacket = carrier * minPhaseEnv * diffNorm;
                clickPhase_ += static_cast<double>(wInst);
            }
            clickSample = (waveletPacket - clickPrevWavelet_) * clickGain * settings.clickLevel;
            clickPrevWavelet_ = waveletPacket;
        }
    }

    // Impact density runs fast (2 ms) log-domain transient compression into 2x oversampled ADAA saturation.
    const float rawAttackSat = std::exp(-mainAge / (0.008f * sr));
    const float attackSat = mainAge >= 0.0f ? rawAttackSat : 0.0f;
    StereoFrame shapedNoise{ rawNoiseL, rawNoiseR };
    if (impact > 0.001f) {
        shapedNoise = impactNoiseSat_.process(rawNoiseL, rawNoiseR, impact * attackSat, 1.15f, sampleRate_);
    }
    const float totalNoiseL = shapedNoise.left * noiseEnv * settings.noiseLevel;
    const float totalNoiseR = shapedNoise.right * noiseEnv * settings.noiseLevel;

    if (std::abs(settings.noiseHPF - lastHPF_) > 0.1f) {
        lastHPF_ = settings.noiseHPF;
        computeHighpassCoeffs(lastHPF_, sampleRate_, b0_h_, b1_h_, b2_h_, a1_h_, a2_h_);
    }
    const float filteredHP_L = processBiquad(
        processBiquad(totalNoiseL, noiseHPF_L_, b0_h_, b1_h_, b2_h_, a1_h_, a2_h_),
        noiseHPF2_L_, b0_h_, b1_h_, b2_h_, a1_h_, a2_h_);
    const float filteredHP_R = processBiquad(
        processBiquad(totalNoiseR, noiseHPF_R_, b0_h_, b1_h_, b2_h_, a1_h_, a2_h_),
        noiseHPF2_R_, b0_h_, b1_h_, b2_h_, a1_h_, a2_h_);

    if (std::abs(settings.noiseLPF - lastLPF_) > 0.1f) {
        lastLPF_ = settings.noiseLPF;
        computeLowpassCoeffs(lastLPF_, sampleRate_, b0_l_, b1_l_, b2_l_, a1_l_, a2_l_);
    }
    float filteredNoiseL = processBiquad(
        processBiquad(filteredHP_L, noiseLPF_L_, b0_l_, b1_l_, b2_l_, a1_l_, a2_l_),
        noiseLPF2_L_, b0_l_, b1_l_, b2_l_, a1_l_, a2_l_);
    float filteredNoiseR = processBiquad(
        processBiquad(filteredHP_R, noiseLPF_R_, b0_l_, b1_l_, b2_l_, a1_l_, a2_l_),
        noiseLPF2_R_, b0_l_, b1_l_, b2_l_, a1_l_, a2_l_);
    if (!ampEnabled_ && mainAge >= 0.0f) {
        const float snapSamples = 0.0022f * static_cast<float>(sampleRate_);
        if (mainAge < snapSamples) {
            const float snap = 1.0f + 2.0f * (1.0f - mainAge / snapSamples);
            filteredNoiseL *= snap;
            filteredNoiseR *= snap;
        }
    }

    float toneSampleL = 0.0f;
    float toneSampleR = 0.0f;
    if (mainAge >= 0.0f && settings.toneLevel > 0.001f) {
        const float pitchDecaySamples = std::max(1.0f, std::clamp(settings.tonePitchDecayMs, 3.0f, 600.0f)
            * 0.001f * static_cast<float>(sampleRate_));
        const float pitchSemitones = std::clamp(settings.tonePitchEnv, -24.0f, 24.0f)
            * std::exp(-std::max(0.0f, mainAge) / pitchDecaySamples);
        const float pitchRatio = std::exp2(pitchSemitones / 12.0f);
        const float nyquistGuard = static_cast<float>(sampleRate_) * 0.48f;
        const float fundamental = std::clamp(midiNoteToHz(midiNote_) * pitchHumanizeRatio_ * pitchRatio,
                                             20.0f, nyquistGuard);
        const float toneQ = std::clamp(settings.toneQ, 0.5f, 12.0f);
        const float detuneL = std::exp2(-8.5f / 1200.0f);
        const float detuneR = std::exp2(9.5f / 1200.0f);
        if (std::abs(fundamental - lastToneFrequency_) > 0.4f || std::abs(toneQ - lastToneQ_) > 0.01f) {
            lastToneFrequency_ = fundamental;
            lastToneQ_ = toneQ;
            const float fundL = std::clamp(fundamental * detuneL, 20.0f, nyquistGuard);
            const float fundR = std::clamp(fundamental * detuneR, 20.0f, nyquistGuard);
            palmModalL_.configure(fundL, toneQ, sampleRate_);
            palmModalR_.configure(fundR, toneQ, sampleRate_);
            // Stereo bloom with opposite formant skew across 900-1700 Hz for crowd-clap depth.
            const float bloomBaseHz = std::clamp(980.0f + fundamental * 0.28f, 900.0f, 1700.0f);
            const float bloomHzL = std::clamp(bloomBaseHz * 0.92f, 900.0f, 1700.0f);
            const float bloomHzR = std::clamp(bloomBaseHz * 1.08f, 900.0f, 1700.0f);
            computeBandpassCoeffs(bloomHzL, 0.72f, sampleRate_, bloomCoeffsL_.b0, bloomCoeffsL_.b1, bloomCoeffsL_.b2, bloomCoeffsL_.a1, bloomCoeffsL_.a2);
            computeBandpassCoeffs(bloomHzR, 0.72f, sampleRate_, bloomCoeffsR_.b0, bloomCoeffsR_.b1, bloomCoeffsR_.b2, bloomCoeffsR_.a1, bloomCoeffsR_.a2);
        }
        const float impulse = mainAge < 1.0f ? 2.2f : 0.0f;
        const float knockEnv = std::exp(-mainAge / std::max(1.0f, 0.006f * sr));
        const float modalDriveWeight = 0.30f + 0.70f * knockEnv;
        const float knockL = palmModalL_.process(impulse + rawNoiseL * modalDriveWeight);
        const float knockR = palmModalR_.process(impulse + rawNoiseR * modalDriveWeight);
        const float bloomMix = 1.0f / (1.0f + 0.12f * std::max(0.0f, toneQ - 2.0f));
        const float bloomL = processBiquad(rawNoiseL, toneBloomFilterL_,
                                           bloomCoeffsL_.b0, bloomCoeffsL_.b1, bloomCoeffsL_.b2, bloomCoeffsL_.a1, bloomCoeffsL_.a2)
            * (0.70f * bloomMix);
        const float bloomR = processBiquad(rawNoiseR, toneBloomFilterR_,
                                           bloomCoeffsR_.b0, bloomCoeffsR_.b1, bloomCoeffsR_.b2, bloomCoeffsR_.a1, bloomCoeffsR_.a2)
            * (0.70f * bloomMix);
        float thumpL = 0.0f;
        float thumpR = 0.0f;
        if (impact > 0.001f && mainAge < 0.050f * sr) {
            const float lpCoeffL = 1.0f - std::exp(-2.0f * pi() * (340.0f * detuneL) / sr);
            const float lpCoeffR = 1.0f - std::exp(-2.0f * pi() * (340.0f * detuneR) / sr);
            impactLpL_ += lpCoeffL * (rawNoiseL - impactLpL_);
            impactLpR_ += lpCoeffR * (rawNoiseR - impactLpR_);
            const float thumpEnv = std::exp(-mainAge / (0.016f * sr));
            thumpL = impactLpL_ * thumpEnv * impact * 2.2f;
            thumpR = impactLpR_ * thumpEnv * impact * 2.2f;
        }
        const float knockGain = 1.28f + 0.45f * std::min(toneQ, 6.0f) / 6.0f;
        toneSampleL = knockL * knockGain + bloomL + thumpL;
        toneSampleR = knockR * knockGain + bloomR + thumpR;
        if (impact > 0.001f) {
            const auto satTone = impactToneSat_.process(toneSampleL, toneSampleR, impact * knockEnv, 0.75f, sampleRate_);
            toneSampleL = satTone.left;
            toneSampleR = satTone.right;
        }
        if (std::abs(settings.toneHPF - lastToneHPF_) > 0.5f || lastToneHPF_ < 0.0f) {
            lastToneHPF_ = settings.toneHPF;
            computeHighpassCoeffs(lastToneHPF_, sampleRate_, b0_th_, b1_th_, b2_th_, a1_th_, a2_th_);
        }
        if (std::abs(settings.toneLPF - lastToneLPF_) > 0.5f || lastToneLPF_ < 0.0f) {
            lastToneLPF_ = settings.toneLPF;
            computeLowpassCoeffs(lastToneLPF_, sampleRate_, b0_tl_, b1_tl_, b2_tl_, a1_tl_, a2_tl_);
        }
        toneSampleL = processBiquad(toneSampleL, toneHPF_L_, b0_th_, b1_th_, b2_th_, a1_th_, a2_th_);
        toneSampleL = processBiquad(toneSampleL, toneLPF_L_, b0_tl_, b1_tl_, b2_tl_, a1_tl_, a2_tl_);
        toneSampleR = processBiquad(toneSampleR, toneHPF_R_, b0_th_, b1_th_, b2_th_, a1_th_, a2_th_);
        toneSampleR = processBiquad(toneSampleR, toneLPF_R_, b0_tl_, b1_tl_, b2_tl_, a1_tl_, a2_tl_);
        const float bodyLevel = toneEnv * settings.toneLevel;
        toneSampleL *= bodyLevel;
        toneSampleR *= bodyLevel;
    }

    if (settings.noiseStereoWidth != 1.0f) {
        const float mid = (filteredNoiseL + filteredNoiseR) * 0.5f;
        const float side = (filteredNoiseL - filteredNoiseR) * 0.5f * settings.noiseStereoWidth;
        filteredNoiseL = mid + side;
        filteredNoiseR = mid - side;
    }

    const float preHighL = processBiquad(
        processBiquad(burstSignalL, preHPF_L_, b0_h_, b1_h_, b2_h_, a1_h_, a2_h_),
        preHPF2_L_, b0_h_, b1_h_, b2_h_, a1_h_, a2_h_);
    const float preHighR = processBiquad(
        processBiquad(burstSignalR, preHPF_R_, b0_h_, b1_h_, b2_h_, a1_h_, a2_h_),
        preHPF2_R_, b0_h_, b1_h_, b2_h_, a1_h_, a2_h_);
    float preLeft = processBiquad(
        processBiquad(preHighL, preLPF_L_, b0_l_, b1_l_, b2_l_, a1_l_, a2_l_),
        preLPF2_L_, b0_l_, b1_l_, b2_l_, a1_l_, a2_l_);
    float preRight = processBiquad(
        processBiquad(preHighR, preLPF_R_, b0_l_, b1_l_, b2_l_, a1_l_, a2_l_),
        preLPF2_R_, b0_l_, b1_l_, b2_l_, a1_l_, a2_l_);
    const float preMid = (preLeft + preRight) * 0.5f;
    const float preSide = (preLeft - preRight) * 0.5f * settings.noiseStereoWidth;
    preLeft = preMid + preSide;
    preRight = preMid - preSide;

    const std::array<StereoFrame, 4> layers{{
        {toneSampleL * layerGains[0], toneSampleR * layerGains[0]},
        {filteredNoiseL * layerGains[1], filteredNoiseR * layerGains[1]},
        {clickSample * layerGains[2], clickSample * layerGains[2]},
        {preLeft * layerGains[3], preRight * layerGains[3]}
    }};
    // Leave the limiter some room. Factory levels are trimmed again on the way out.
    constexpr float kHeadroom = 0.12f;
    std::array<StereoFrame, 4> scaled{};
    for (size_t layer = 0; layer < layers.size(); ++layer) {
        scaled[layer].left = layers[layer].left * kHeadroom;
        scaled[layer].right = layers[layer].right * kHeadroom;
    }
    if (!ampEnabled_ && totalDurationSamples_ > 0) {
        const int fadeSamples = std::max(1, static_cast<int>(0.004 * sampleRate_));
        const int remaining = totalDurationSamples_ - static_cast<int>(currentSample);
        if (remaining < fadeSamples) {
            const float fade = std::clamp(static_cast<float>(remaining) / static_cast<float>(fadeSamples), 0.0f, 1.0f);
            for (auto& layer : scaled) {
                layer.left *= fade;
                layer.right *= fade;
            }
        }
    }
    if (components != nullptr) *components = scaled;
    StereoFrame mix;
    for (const auto& layer : scaled) {
        mix.left += layer.left;
        mix.right += layer.right;
    }
    if (!std::isfinite(mix.left) || !std::isfinite(mix.right)) {
        palmModalL_.reset();
        palmModalR_.reset();
        toneBloomFilterL_.reset();
        toneBloomFilterR_.reset();
        clickPhase_ = 0.0;
        clickPrevWavelet_ = 0.0f;
        return {};
    }
    return mix;
}

void ClapSynth::renderBlock(
    std::vector<float>& leftOut,
    std::vector<float>& rightOut,
    const ClapSynthSettings& settings,
    float velocity,
    int midiNote) noexcept {
    trigger(settings, midiNote, velocity);

    leftOut.resize(totalDurationSamples_);
    rightOut.resize(totalDurationSamples_);

    for (int i = 0; i < totalDurationSamples_; ++i) {
        const auto frame = processSample(settings);
        leftOut[i] = frame.left;
        rightOut[i] = frame.right;
    }
}

} // namespace openclapper::core
