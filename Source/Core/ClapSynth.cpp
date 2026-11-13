#include "ClapSynth.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace openclapper::core {

namespace {

constexpr std::array<float, 8> kBandpassPartials = { 1.0f, 0.34f, 0.18f, 0.10f, 0.06f, 0.03f, 0.015f, 0.008f };
constexpr std::array<float, 8> kSubThumpPartials = { 1.0f, 0.08f, 0.02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
constexpr std::array<float, 8> kSaturatedTriPartials = { 1.0f, 0.0f, 0.16f, 0.0f, 0.07f, 0.0f, 0.035f, 0.0f };
constexpr std::array<float, 8> kCyberSquarePartials = { 1.0f, 0.42f, 0.26f, 0.18f, 0.12f, 0.08f, 0.05f, 0.03f };

[[nodiscard]] constexpr float pi() noexcept {
    return std::numbers::pi_v<float>;
}

// Fast xorshift32 PRNG
[[nodiscard]] inline float nextRandomFloat(uint32_t& state) noexcept {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return (static_cast<float>(state) * (2.0f / 4294967296.0f)) - 1.0f;
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

// ============================================================================
// GatedStereoReverb
// ============================================================================

float GatedStereoReverb::readHermite(const std::vector<float>& buffer, double delaySamples, int writePos) noexcept {
    const double size = static_cast<double>(buffer.size());
    double readPos = static_cast<double>(writePos) - delaySamples;
    while (readPos < 0.0) readPos += size;
    while (readPos >= size) readPos -= size;

    const int i1 = static_cast<int>(readPos);
    const float frac = static_cast<float>(readPos - static_cast<double>(i1));
    const int i0 = (i1 - 1 + static_cast<int>(buffer.size())) % static_cast<int>(buffer.size());
    const int i2 = (i1 + 1) % static_cast<int>(buffer.size());
    const int i3 = (i1 + 2) % static_cast<int>(buffer.size());

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

void GatedStereoReverb::prepare(double sampleRate) {
    diffL1_.prepare(std::max(8, static_cast<int>(sampleRate * 0.0031)), 0.65f);
    diffL2_.prepare(std::max(8, static_cast<int>(sampleRate * 0.0079)), 0.60f);
    diffL3_.prepare(std::max(8, static_cast<int>(sampleRate * 0.0113)), 0.55f);
    diffR1_.prepare(std::max(8, static_cast<int>(sampleRate * 0.0023)), 0.65f);
    diffR2_.prepare(std::max(8, static_cast<int>(sampleRate * 0.0058)), 0.60f);
    diffR3_.prepare(std::max(8, static_cast<int>(sampleRate * 0.0097)), 0.55f);

    const int margin = static_cast<int>(sampleRate * 0.003); // headroom for LFO modulation
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
    dampStateL1_ = dampStateL2_ = 0.0f;
    dampStateR1_ = dampStateR2_ = 0.0f;
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

    // Compute gate envelope with smooth exponential-release damping
    if (gateMs > 0.0f) {
        const float gateSamples = (gateMs * 0.001f) * static_cast<float>(sampleRate);
        const float releaseSamples = (18.0f * 0.001f) * static_cast<float>(sampleRate);
        if (static_cast<float>(sampleCounter_) > gateSamples) {
            const float samplesOver = static_cast<float>(sampleCounter_) - gateSamples;
            gateEnvelope_ = std::max(0.0f, 1.0f - (samplesOver / releaseSamples));
        } else {
            gateEnvelope_ = 1.0f;
        }
    } else {
        gateEnvelope_ = 1.0f;
    }

    // Input highpass filter (140 Hz) to keep the reverb tank clean from low rumble
    const float hpfCoeff = static_cast<float>(1.0 - std::exp(-2.0 * pi() * 140.0 / sampleRate));
    hpfStateL_ += hpfCoeff * (inLeft - hpfStateL_);
    const float cleanInL = inLeft - hpfStateL_;
    hpfStateR_ += hpfCoeff * (inRight - hpfStateR_);
    const float cleanInR = inRight - hpfStateR_;

    // Multi-stage allpass diffuser cascade for dense transient dispersion
    const float diffInL = diffL3_.process(diffL2_.process(diffL1_.process(cleanInL)));
    const float diffInR = diffR3_.process(diffR2_.process(diffR1_.process(cleanInR)));

    const float feedback = std::clamp(std::pow(0.001f, 30.0f / std::max(10.0f, decayMs)), 0.1f, 0.94f) * (0.6f + 0.4f * size);
    const float dampFactor = std::clamp(damp, 0.0f, 0.85f);

    // LFO modulations to disperse metallic room modes and flutter
    lfoPhaseL_ += 2.0 * pi() * 0.73 / sampleRate;
    if (lfoPhaseL_ > 2.0 * pi()) lfoPhaseL_ -= 2.0 * pi();
    lfoPhaseR_ += 2.0 * pi() * 0.91 / sampleRate;
    if (lfoPhaseR_ > 2.0 * pi()) lfoPhaseR_ -= 2.0 * pi();

    const double modL = std::sin(lfoPhaseL_) * (0.0008 * sampleRate);
    const double modR = std::cos(lfoPhaseR_) * (0.0008 * sampleRate);

    const double baseL1 = sampleRate * 0.0297;
    const double baseL2 = sampleRate * 0.0371;
    const double baseL3 = sampleRate * 0.0439;
    const double baseR1 = sampleRate * 0.0313;
    const double baseR2 = sampleRate * 0.0419;
    const double baseR3 = sampleRate * 0.0487;

    // Hermite interpolated delay line reads
    const float outL1 = readHermite(delayL1_, baseL1 + modL, writeL1_);
    const float outL2 = readHermite(delayL2_, baseL2 - modR, writeL2_);
    const float outL3 = readHermite(delayL3_, baseL3 + modR * 0.5, writeL3_);

    const float outR1 = readHermite(delayR1_, baseR1 + modR, writeR1_);
    const float outR2 = readHermite(delayR2_, baseR2 - modL, writeR2_);
    const float outR3 = readHermite(delayR3_, baseR3 - modL * 0.5, writeR3_);

    // Cross-coupled matrix
    const float wetL = 0.35f * (outL1 + outL2 + outL3);
    const float wetR = 0.35f * (outR1 + outR2 + outR3);

    dampStateL1_ = dampStateL1_ * dampFactor + wetL * (1.0f - dampFactor);
    dampStateR1_ = dampStateR1_ * dampFactor + wetR * (1.0f - dampFactor);

    // Unitary cross-feedback injection
    delayL1_[static_cast<size_t>(writeL1_)] = diffInL + dampStateR1_ * feedback;
    delayL2_[static_cast<size_t>(writeL2_)] = diffInL * 0.65f + dampStateL1_ * feedback * 0.75f;
    delayL3_[static_cast<size_t>(writeL3_)] = diffInL * 0.5f + (dampStateR1_ - dampStateL1_) * feedback * 0.45f;

    delayR1_[static_cast<size_t>(writeR1_)] = diffInR + dampStateL1_ * feedback;
    delayR2_[static_cast<size_t>(writeR2_)] = diffInR * 0.65f + dampStateR1_ * feedback * 0.75f;
    delayR3_[static_cast<size_t>(writeR3_)] = diffInR * 0.5f + (dampStateL1_ - dampStateR1_) * feedback * 0.45f;

    writeL1_ = (writeL1_ + 1) % static_cast<int>(delayL1_.size());
    writeL2_ = (writeL2_ + 1) % static_cast<int>(delayL2_.size());
    writeL3_ = (writeL3_ + 1) % static_cast<int>(delayL3_.size());
    writeR1_ = (writeR1_ + 1) % static_cast<int>(delayR1_.size());
    writeR2_ = (writeR2_ + 1) % static_cast<int>(delayR2_.size());
    writeR3_ = (writeR3_ + 1) % static_cast<int>(delayR3_.size());

    const float outLeft = (wetL - wetR * 0.25f) * gateEnvelope_;
    const float outRight = (wetR - wetL * 0.25f) * gateEnvelope_;
    return { outLeft, outRight };
}

// ============================================================================
// ClapSynth
// ============================================================================

void ClapSynth::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
    reset();
}

void ClapSynth::reset() noexcept {
    isPlaying_ = false;
    totalSamplesPlayed_ = 0;
    totalDurationSamples_ = 0;
    burstCount_ = 0;
    partialFrequencies_.fill(0.0f);
    partialPhases_.fill(0.0f);
    lastHPF_ = -1.0f;
    lastLPF_ = -1.0f;
    noiseHPF_L_.reset();
    noiseHPF_R_.reset();
    noiseLPF_L_.reset();
    noiseLPF_R_.reset();
    pinkB0_ = pinkB1_ = pinkB2_ = 0.0f;
    lofiHeld_ = 0.0f;
    lofiCounter_ = 0;
}

void ClapSynth::trigger(const ClapSynthSettings& settings, int midiNote, float velocity) noexcept {
    reset();
    isPlaying_ = true;
    velocity_ = std::clamp(velocity, 0.0f, 1.0f);
    midiNote_ = std::clamp(midiNote, 0, 127);
    ++triggerCount_;
    prngState_ = 0x9e3779b9U ^ (static_cast<uint32_t>(midiNote_) * 0x85ebca6bU) ^ (triggerCount_ * 0xc2b2ae35U);
    if (prngState_ == 0) {
        prngState_ = 0x12345678U;
    }

    burstCount_ = std::clamp(settings.preClapCount, 0, 5);
    const float spreadSamples = std::max(0.0f, settings.preClapSpreadMs) * 0.001f * static_cast<float>(sampleRate_);

    for (int i = 0; i < burstCount_; ++i) {
        const float jitter = std::clamp(settings.preClapJitter, 0.0f, 1.0f);
        const float jitterTime = nextRandomFloat(prngState_) * 0.5f * jitter * spreadSamples;
        burstStartSamples_[i] = std::max(0, static_cast<int>(static_cast<float>(i + 1) * spreadSamples + jitterTime));
        const float jitterAmp = 1.0f + nextRandomFloat(prngState_) * 0.2f * jitter;
        burstAmplitudes_[i] = std::clamp(settings.preClapLevel * jitterAmp, 0.0f, 1.0f);
        const float dir = (i % 2 == 0) ? -1.0f : 1.0f;
        const float spreadFrac = static_cast<float>(i + 1) / static_cast<float>(std::max(1, burstCount_));
        burstPans_[i] = std::clamp(dir * settings.preClapFlamSpread * spreadFrac, -1.0f, 1.0f);
    }

    const float fundamentalHz = midiNoteToHz(midiNote_);
    for (size_t i = 0; i < partialFrequencies_.size(); ++i) {
        const float partialHz = fundamentalHz * static_cast<float>(i + 1);
        partialFrequencies_[i] = partialHz < static_cast<float>(sampleRate_) * 0.48f ? partialHz : 0.0f;
    }

    const float burstTailMs = burstCount_ > 0
        ? static_cast<float>(burstCount_) * settings.preClapSpreadMs + settings.preClapDecayMs * 4.0f
        : 0.0f;
    const float noiseTailMs = settings.noiseAttackMs + settings.noiseHoldMs + settings.noiseDecayMs * 4.0f;
    const float totalTimeMs = std::max({ burstTailMs, noiseTailMs, settings.toneDecayMs * 6.0f, 20.0f });
    totalDurationSamples_ = static_cast<int>(totalTimeMs * 0.001f * static_cast<float>(sampleRate_));
}

float ClapSynth::generateNoiseSample(NoiseType type) noexcept {
    const float white = nextRandomFloat(prngState_);

    if (type == NoiseType::Pink) {
        pinkB0_ = 0.99765f * pinkB0_ + white * 0.0990460f;
        pinkB1_ = 0.96300f * pinkB1_ + white * 0.2965164f;
        pinkB2_ = 0.57000f * pinkB2_ + white * 1.0526913f;
        return (pinkB0_ + pinkB1_ + pinkB2_ + white * 0.1848f) * 0.35f;
    }

    if (type == NoiseType::Velvet) {
        // Sparse velvet noise pulses (density ~25%)
        const float r = nextRandomFloat(prngState_);
        if (std::abs(r) > 0.65f) {
            return (r > 0.0f ? 1.0f : -1.0f) * (0.8f + 0.2f * std::abs(white));
        }
        return 0.0f;
    }

    if (type == NoiseType::LoFi) {
        // Decimated 8-bit sample-and-hold
        if (++lofiCounter_ >= 4) {
            lofiCounter_ = 0;
            // 8-bit quantization
            lofiHeld_ = std::round(white * 15.0f) / 15.0f;
        }
        return lofiHeld_;
    }

    return white; // Default White
}

float ClapSynth::getEnvelopeProgress() const noexcept {
    if (!isPlaying_ || totalDurationSamples_ <= 0) {
        return 1.0f;
    }
    return std::clamp(static_cast<float>(totalSamplesPlayed_) / static_cast<float>(totalDurationSamples_), 0.0f, 1.0f);
}

StereoFrame ClapSynth::processSample(const ClapSynthSettings& settings) noexcept {
    if (!isPlaying_) {
        return { 0.0f, 0.0f };
    }

    const int currentSample = totalSamplesPlayed_++;
    if (currentSample >= totalDurationSamples_) {
        isPlaying_ = false;
        return { 0.0f, 0.0f };
    }

    float burstGainL = 0.0f;
    float burstGainR = 0.0f;
    const float burstDecaySamples = std::max(1.0f, settings.preClapDecayMs * 0.001f * static_cast<float>(sampleRate_));
    for (int i = 0; i < burstCount_; ++i) {
        const int age = currentSample - burstStartSamples_[i];
        if (age < 0 || static_cast<float>(age) >= burstDecaySamples * 4.0f) {
            continue;
        }

        const float env = std::exp(-static_cast<float>(age) / burstDecaySamples);
        const float pan = burstPans_[i];
        const float gainL = std::clamp(0.5f * (1.0f - pan), 0.0f, 1.0f);
        const float gainR = std::clamp(0.5f * (1.0f + pan), 0.0f, 1.0f);
        const float amplitude = env * burstAmplitudes_[i] * 1.414f;
        burstGainL += amplitude * gainL;
        burstGainR += amplitude * gainR;
    }

    const float mainAge = static_cast<float>(currentSample);
    const float attackSamples = std::max(0.0f, settings.noiseAttackMs * 0.001f * static_cast<float>(sampleRate_));
    const float holdSamples = std::max(0.0f, settings.noiseHoldMs * 0.001f * static_cast<float>(sampleRate_));
    const float decaySamples = std::max(1.0f, settings.noiseDecayMs * 0.001f * static_cast<float>(sampleRate_));
    float noiseEnv = 0.0f;
    if (mainAge < attackSamples && attackSamples > 0.0f) {
        noiseEnv = mainAge / attackSamples;
    } else if (mainAge < attackSamples + holdSamples) {
        noiseEnv = 1.0f;
    } else {
        const float decayT = mainAge - attackSamples - holdSamples;
        const float linearFraction = std::clamp(decayT / decaySamples, 0.0f, 1.0f);
        const float expDecay = std::exp(-decayT / std::max(1.0f, decaySamples * 0.35f));
        const float curve = std::clamp(settings.noiseCurve, -1.0f, 1.0f);
        if (curve < 0.0f) {
            noiseEnv = (1.0f + curve) * (1.0f - linearFraction) - curve * expDecay;
        } else {
            noiseEnv = (1.0f - curve) * expDecay + curve * (1.0f - linearFraction);
        }
    }

    const float toneDecaySamples = std::max(1.0f, settings.toneDecayMs * 0.001f * static_cast<float>(sampleRate_));
    const float toneEnv = std::exp(-mainAge / toneDecaySamples);
    float clickSample = 0.0f;
    if (mainAge < sampleRate_ * 0.005f) {
        const float clickT = mainAge / static_cast<float>(sampleRate_);
        const float clickHz = std::min(settings.clickFreqHz, static_cast<float>(sampleRate_) * 0.45f);
        const float envFast = std::exp(-mainAge / (0.0008f * static_cast<float>(sampleRate_)));
        switch (settings.clickType) {
            case ClickType::Needle: {
                clickSample = std::sin(2.0f * pi() * clickHz * clickT) * envFast * settings.clickLevel;
                break;
            }
            case ClickType::Crack: {
                const float phase1 = 2.0f * pi() * clickHz * clickT;
                const float phase2 = 2.0f * pi() * std::min(clickHz * 1.6f, static_cast<float>(sampleRate_) * 0.45f) * clickT;
                clickSample = (std::sin(phase1) * 0.6f + std::sin(phase2) * 0.4f) * envFast * settings.clickLevel * 1.3f;
                break;
            }
            case ClickType::Thump: {
                clickSample = std::sin(2.0f * pi() * clickHz * 0.45f * clickT)
                    * std::exp(-mainAge / (0.002f * static_cast<float>(sampleRate_)))
                    * settings.clickLevel * 1.2f;
                break;
            }
        }
    }

    const float rawNoiseL = generateNoiseSample(settings.noiseType);
    const float rawNoiseR = generateNoiseSample(settings.noiseType);
    const float totalNoiseL = (rawNoiseL * burstGainL + rawNoiseL * noiseEnv * settings.noiseLevel) * velocity_;
    const float totalNoiseR = (rawNoiseR * burstGainR + rawNoiseR * noiseEnv * settings.noiseLevel) * velocity_;

    if (std::abs(settings.noiseHPF - lastHPF_) > 0.1f) {
        lastHPF_ = settings.noiseHPF;
        computeHighpassCoeffs(lastHPF_, sampleRate_, b0_h_, b1_h_, b2_h_, a1_h_, a2_h_);
    }
    const float filteredHP_L = processBiquad(totalNoiseL, noiseHPF_L_, b0_h_, b1_h_, b2_h_, a1_h_, a2_h_);
    const float filteredHP_R = processBiquad(totalNoiseR, noiseHPF_R_, b0_h_, b1_h_, b2_h_, a1_h_, a2_h_);

    if (std::abs(settings.noiseLPF - lastLPF_) > 0.1f) {
        lastLPF_ = settings.noiseLPF;
        computeLowpassCoeffs(lastLPF_, sampleRate_, b0_l_, b1_l_, b2_l_, a1_l_, a2_l_);
    }
    float filteredNoiseL = processBiquad(filteredHP_L, noiseLPF_L_, b0_l_, b1_l_, b2_l_, a1_l_, a2_l_);
    float filteredNoiseR = processBiquad(filteredHP_R, noiseLPF_R_, b0_l_, b1_l_, b2_l_, a1_l_, a2_l_);

    float toneSample = 0.0f;
    if (settings.toneLevel > 0.001f) {
        const std::array<float, 8>* weights = &kBandpassPartials;
        switch (settings.toneMode) {
            case ToneMode::Bandpass: weights = &kBandpassPartials; break;
            case ToneMode::SubThump: weights = &kSubThumpPartials; break;
            case ToneMode::SaturatedTri: weights = &kSaturatedTriPartials; break;
            case ToneMode::CyberSquare: weights = &kCyberSquarePartials; break;
        }

        const float resonance = std::clamp(settings.toneQ / 6.0f, 0.1f, 2.0f);
        float weightSum = 0.0f;
        for (size_t i = 0; i < partialFrequencies_.size(); ++i) {
            const float frequency = partialFrequencies_[i];
            if (frequency <= 0.0f) {
                continue;
            }

            partialPhases_[i] += 2.0f * pi() * frequency / static_cast<float>(sampleRate_);
            if (partialPhases_[i] >= 2.0f * pi()) {
                partialPhases_[i] -= 2.0f * pi();
            }
            const float highPartial = static_cast<float>(i) / static_cast<float>(partialFrequencies_.size() - 1);
            const float weight = (*weights)[i] * (1.0f + (resonance - 1.0f) * highPartial);
            toneSample += std::sin(partialPhases_[i]) * weight;
            weightSum += weight;
        }
        toneSample = toneSample / std::max(1.0f, weightSum) * toneEnv * settings.toneLevel * velocity_;
    }

    if (settings.noiseStereoWidth != 1.0f) {
        const float mid = (filteredNoiseL + filteredNoiseR) * 0.5f;
        const float side = (filteredNoiseL - filteredNoiseR) * 0.5f * settings.noiseStereoWidth;
        filteredNoiseL = mid + side;
        filteredNoiseR = mid - side;
    }

    const float click = clickSample * velocity_;
    return { filteredNoiseL + toneSample + click, filteredNoiseR + toneSample + click };
}

void ClapSynth::renderBlock(
    std::vector<float>& leftOut,
    std::vector<float>& rightOut,
    const ClapSynthSettings& settings,
    float velocity,
    int semitoneOffset) noexcept {
    trigger(settings, 69 + semitoneOffset, velocity);

    leftOut.resize(totalDurationSamples_);
    rightOut.resize(totalDurationSamples_);

    for (int i = 0; i < totalDurationSamples_; ++i) {
        const auto frame = processSample(settings);
        leftOut[i] = frame.left;
        rightOut[i] = frame.right;
    }
}

} // namespace openclapper::core
