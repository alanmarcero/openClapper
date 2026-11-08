#include "ClapSynth.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace openclapper::core {

namespace {

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

// Biquad bandpass filter coefficients (constant 0 dB peak gain)
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

void GatedStereoReverb::prepare(double sampleRate) {
    diffL1_.prepare(std::max(8, static_cast<int>(sampleRate * 0.0031)), 0.65f);
    diffL2_.prepare(std::max(8, static_cast<int>(sampleRate * 0.0079)), 0.60f);
    diffR1_.prepare(std::max(8, static_cast<int>(sampleRate * 0.0023)), 0.65f);
    diffR2_.prepare(std::max(8, static_cast<int>(sampleRate * 0.0058)), 0.60f);

    delayL1_.assign(std::max(16, static_cast<int>(sampleRate * 0.0297)), 0.0f);
    delayL2_.assign(std::max(16, static_cast<int>(sampleRate * 0.0371)), 0.0f);
    delayR1_.assign(std::max(16, static_cast<int>(sampleRate * 0.0313)), 0.0f);
    delayR2_.assign(std::max(16, static_cast<int>(sampleRate * 0.0419)), 0.0f);
    reset();
}

void GatedStereoReverb::reset() noexcept {
    diffL1_.reset();
    diffL2_.reset();
    diffR1_.reset();
    diffR2_.reset();
    std::fill(delayL1_.begin(), delayL1_.end(), 0.0f);
    std::fill(delayL2_.begin(), delayL2_.end(), 0.0f);
    std::fill(delayR1_.begin(), delayR1_.end(), 0.0f);
    std::fill(delayR2_.begin(), delayR2_.end(), 0.0f);
    writeL1_ = writeL2_ = writeR1_ = writeR2_ = 0;
    dampStateL_ = dampStateR_ = 0.0f;
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

    // Compute gate envelope
    if (gateMs > 0.0f) {
        const float gateSamples = (gateMs * 0.001f) * static_cast<float>(sampleRate);
        const float releaseSamples = (15.0f * 0.001f) * static_cast<float>(sampleRate);
        if (static_cast<float>(sampleCounter_) > gateSamples) {
            const float samplesOver = static_cast<float>(sampleCounter_) - gateSamples;
            gateEnvelope_ = std::max(0.0f, 1.0f - (samplesOver / releaseSamples));
        } else {
            gateEnvelope_ = 1.0f;
        }
    } else {
        gateEnvelope_ = 1.0f;
    }

    if (gateEnvelope_ <= 0.0f) {
        return { 0.0f, 0.0f };
    }

    // Disperse transient impulses with allpass diffuser stages
    const float diffInL = diffL2_.process(diffL1_.process(inLeft));
    const float diffInR = diffR2_.process(diffR1_.process(inRight));

    const float feedback = std::clamp(std::pow(0.001f, 30.0f / std::max(10.0f, decayMs)), 0.1f, 0.94f) * (0.6f + 0.4f * size);
    const float dampFactor = std::clamp(damp, 0.0f, 0.85f);

    // Read delay lines
    const float outL1 = delayL1_[writeL1_];
    const float outL2 = delayL2_[writeL2_];
    const float outR1 = delayR1_[writeR1_];
    const float outR2 = delayR2_[writeR2_];

    // Cross-coupled matrix
    const float wetL = 0.5f * (outL1 + outL2);
    const float wetR = 0.5f * (outR1 + outR2);

    dampStateL_ = dampStateL_ * dampFactor + wetL * (1.0f - dampFactor);
    dampStateR_ = dampStateR_ * dampFactor + wetR * (1.0f - dampFactor);

    // Write back with cross feedback
    delayL1_[writeL1_] = diffInL + dampStateR_ * feedback;
    delayL2_[writeL2_] = diffInL * 0.7f + dampStateL_ * feedback * 0.8f;

    delayR1_[writeR1_] = diffInR + dampStateL_ * feedback;
    delayR2_[writeR2_] = diffInR * 0.7f + dampStateR_ * feedback * 0.8f;

    writeL1_ = (writeL1_ + 1) % static_cast<int>(delayL1_.size());
    writeL2_ = (writeL2_ + 1) % static_cast<int>(delayL2_.size());
    writeR1_ = (writeR1_ + 1) % static_cast<int>(delayR1_.size());
    writeR2_ = (writeR2_ + 1) % static_cast<int>(delayR2_.size());

    return { (wetL - wetR * 0.3f) * gateEnvelope_, (wetR - wetL * 0.3f) * gateEnvelope_ };
}

// ============================================================================
// ClapSynth
// ============================================================================

void ClapSynth::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
    reverb_.prepare(sampleRate);
    const int maxDelay = static_cast<int>(sampleRate * 0.05); // 50 ms max stereo delay
    widthDelayBuffer_.assign(std::max(64, maxDelay), 0.0f);
    reset();
}

void ClapSynth::reset() noexcept {
    isPlaying_ = false;
    totalSamplesPlayed_ = 0;
    totalDurationSamples_ = 0;
    tonePhase_ = 0.0f;
    lastHPF_ = -1.0f;
    lastLPF_ = -1.0f;
    toneFilterL_.reset();
    toneFilterR_.reset();
    noiseHPF_L_.reset();
    noiseHPF_R_.reset();
    noiseLPF_L_.reset();
    noiseLPF_R_.reset();
    clickBPF_.reset();
    reverb_.reset();
    pinkB0_ = pinkB1_ = pinkB2_ = 0.0f;
    lofiHeld_ = 0.0f;
    lofiCounter_ = 0;
    std::fill(widthDelayBuffer_.begin(), widthDelayBuffer_.end(), 0.0f);
    widthDelayWritePos_ = 0;
}

void ClapSynth::trigger(const ClapSynthSettings& settings, float velocity, int semitoneOffset) noexcept {
    reset();
    isPlaying_ = true;
    velocity_ = std::clamp(velocity, 0.0f, 1.0f);
    semitoneOffset_ = semitoneOffset;

    preClapCount_ = std::clamp(settings.preClapCount, 1, 5);
    const float spreadSamples = (settings.preClapSpreadMs * 0.001f) * static_cast<float>(sampleRate_);

    // Set micro-burst start times, humanized amplitudes, and micro-panning
    for (int i = 0; i < preClapCount_; ++i) {
        const float jitterTime = (nextRandomFloat(prngState_) * 0.5f) * settings.preClapJitter * spreadSamples;
        burstStartSamples_[i] = std::max(0, static_cast<int>(static_cast<float>(i) * spreadSamples + jitterTime));
        const float jitterAmp = 1.0f + nextRandomFloat(prngState_) * 0.2f * settings.preClapJitter;
        burstAmplitudes_[i] = std::clamp(settings.preClapLevel * jitterAmp, 0.1f, 1.0f);

        // Alternating micro-panning spread across stereo field
        const float dir = (i % 2 == 0) ? -1.0f : 1.0f;
        const float spreadFrac = (preClapCount_ > 1) ? static_cast<float>(i + 1) / static_cast<float>(preClapCount_) : 0.5f;
        burstPans_[i] = std::clamp(dir * settings.preClapFlamSpread * spreadFrac, -1.0f, 1.0f);
    }

    // Main collective slap hit occurs right after the pre-bursts
    mainSlapSample_ = static_cast<int>(static_cast<float>(preClapCount_) * spreadSamples);

    // Calculate total duration to cover decay and reverb tail
    const float totalTimeMs = settings.preClapSpreadMs * static_cast<float>(preClapCount_)
        + std::max({ settings.noiseDecayMs, settings.toneDecayMs, settings.reverbDecayMs, settings.reverbGateMs });
    totalDurationSamples_ = static_cast<int>((totalTimeMs * 0.001f + 0.1f) * static_cast<float>(sampleRate_));
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
    if (totalSamplesPlayed_ >= totalDurationSamples_) {
        isPlaying_ = false;
        return { 0.0f, 0.0f };
    }

    // 1. Evaluate Pre-Clap Micro-bursts with alternating stereo micro-panning
    float preClapGainL = 0.0f;
    float preClapGainR = 0.0f;
    const float effectiveBurstDecayMs = std::max(2.0f, std::min(settings.preClapDecayMs, settings.preClapSpreadMs * 0.75f));
    const float burstDecaySamples = (effectiveBurstDecayMs * 0.001f) * static_cast<float>(sampleRate_);

    for (int i = 0; i < preClapCount_; ++i) {
        const int age = currentSample - burstStartSamples_[i];
        if (age >= 0 && static_cast<float>(age) < burstDecaySamples * 4.0f) {
            const float env = std::exp(-static_cast<float>(age) / std::max(1.0f, burstDecaySamples));
            const float pan = burstPans_[i]; // -1 to +1
            const float gainL = std::clamp(0.5f * (1.0f - pan), 0.0f, 1.0f);
            const float gainR = std::clamp(0.5f * (1.0f + pan), 0.0f, 1.0f);
            const float amp = env * burstAmplitudes_[i];
            preClapGainL += amp * gainL * 1.414f;
            preClapGainR += amp * gainR * 1.414f;
        }
    }

    // 2. Evaluate Main Slap Envelopes
    const int mainAge = currentSample - mainSlapSample_;
    float noiseEnv = 0.0f;
    float toneEnv = 0.0f;
    float clickSample = 0.0f;

    if (mainAge >= 0) {
        const float attackSamples = (settings.noiseAttackMs * 0.001f) * static_cast<float>(sampleRate_);
        const float holdSamples = (settings.noiseHoldMs * 0.001f) * static_cast<float>(sampleRate_);
        const float decaySamples = (settings.noiseDecayMs * 0.001f) * static_cast<float>(sampleRate_);
        const float toneDecaySamples = (settings.toneDecayMs * 0.001f) * static_cast<float>(sampleRate_);

        // Main noise envelope with attack, hold, shaped decay
        const float t = static_cast<float>(mainAge);
        if (t < attackSamples && attackSamples > 0.0f) {
            noiseEnv = t / attackSamples;
        } else if (t < (attackSamples + holdSamples)) {
            noiseEnv = 1.0f;
        } else {
            const float decayT = t - (attackSamples + holdSamples);
            const float linearFraction = std::clamp(decayT / std::max(1.0f, decaySamples), 0.0f, 1.0f);
            const float expDecay = std::exp(-decayT / std::max(1.0f, decaySamples * 0.35f));
            if (settings.noiseCurve < 0.0f) {
                const float factor = -settings.noiseCurve;
                noiseEnv = (1.0f - factor) * (1.0f - linearFraction) + factor * expDecay;
            } else {
                const float factor = settings.noiseCurve;
                noiseEnv = (1.0f - factor) * expDecay + factor * (1.0f - linearFraction);
            }
        }

        // Resonant body tone envelope
        toneEnv = std::exp(-t / std::max(1.0f, toneDecaySamples));

        // Click spike / transient crack
        if (mainAge < static_cast<int>(sampleRate_ * 0.005)) {
            const float clickT = static_cast<float>(mainAge) / static_cast<float>(sampleRate_);
            const float envFast = std::exp(-static_cast<float>(mainAge) / (0.0008f * static_cast<float>(sampleRate_)));
            switch (settings.clickType) {
                case ClickType::Needle: {
                    const float clickPhase = 2.0f * pi() * settings.clickFreqHz * clickT;
                    clickSample = std::sin(clickPhase) * envFast * settings.clickLevel;
                    break;
                }
                case ClickType::Crack: {
                    const float clickPhase1 = 2.0f * pi() * settings.clickFreqHz * clickT;
                    const float clickPhase2 = 2.0f * pi() * (settings.clickFreqHz * 1.6f) * clickT;
                    clickSample = (std::sin(clickPhase1) * 0.6f + std::sin(clickPhase2) * 0.4f) * envFast * settings.clickLevel * 1.3f;
                    break;
                }
                case ClickType::Thump: {
                    const float clickPhase = 2.0f * pi() * (settings.clickFreqHz * 0.45f) * clickT;
                    clickSample = std::sin(clickPhase) * std::exp(-static_cast<float>(mainAge) / (0.002f * static_cast<float>(sampleRate_))) * settings.clickLevel * 1.2f;
                    break;
                }
            }
        }
    }

    // Combine raw noise for pre-claps and main slap
    const float rawNoiseL = generateNoiseSample(settings.noiseType);
    const float rawNoiseR = generateNoiseSample(settings.noiseType);

    // Apply pre-clap flam + main slap noise envelope
    const float totalNoiseL = (rawNoiseL * preClapGainL + rawNoiseL * noiseEnv * settings.noiseLevel) * velocity_;
    const float totalNoiseR = (rawNoiseR * preClapGainR + rawNoiseR * noiseEnv * settings.noiseLevel) * velocity_;

    // Filter noise with HPF and LPF (cached to avoid redundant trigonometric evaluation)
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

    // 3. Resonant Body / Tone synthesis
    float toneL = 0.0f;
    float toneR = 0.0f;
    if (mainAge >= 0 && settings.toneLevel > 0.001f) {
        // Pitch drop envelope
        const float pitchDecaySamples = (settings.tonePitchDecayMs * 0.001f) * static_cast<float>(sampleRate_);
        const float pitchEnvFrac = std::exp(-static_cast<float>(mainAge) / std::max(1.0f, pitchDecaySamples));
        const float pitchSemitones = static_cast<float>(semitoneOffset_) + settings.tonePitchEnv * pitchEnvFrac;

        // Base tuned pitch computed from Note (0..11), Octave (1..8), and Fine cents (-50..+50)
        const float baseHz = noteToHz(settings.note, settings.octave, settings.fineCents);
        const float tunedHz = std::clamp(baseHz * std::pow(2.0f, pitchSemitones / 12.0f), 20.0f, static_cast<float>(sampleRate_) * 0.48f);

        float toneSample = 0.0f;
        tonePhase_ += 2.0f * pi() * tunedHz / static_cast<float>(sampleRate_);
        if (tonePhase_ >= 2.0f * pi()) {
            tonePhase_ = std::fmod(tonePhase_, 2.0f * pi());
        }

        switch (settings.toneMode) {
            case ToneMode::Bandpass: {
                float b0_t, b1_t, b2_t, a1_t, a2_t;
                computeBandpassCoeffs(tunedHz, settings.toneQ, sampleRate_, b0_t, b1_t, b2_t, a1_t, a2_t);
                const float exciter = (mainAge < 4 ? 1.0f : 0.0f) + rawNoiseL * 0.35f;
                toneSample = processBiquad(exciter * toneEnv, toneFilterL_, b0_t, b1_t, b2_t, a1_t, a2_t) * 2.2f;
                break;
            }
            case ToneMode::SubThump: {
                const float sine = std::sin(tonePhase_);
                toneSample = sine * toneEnv * 1.6f;
                break;
            }
            case ToneMode::SaturatedTri: {
                const float tri = (std::asin(std::sin(tonePhase_)) * (2.0f / pi()));
                toneSample = std::tanh(tri * 2.0f) * toneEnv * 1.5f;
                break;
            }
            case ToneMode::CyberSquare: {
                const float sq = (std::sin(tonePhase_) >= 0.0f ? 0.75f : -0.75f);
                float b0_t, b1_t, b2_t, a1_t, a2_t;
                computeLowpassCoeffs(std::min(tunedHz * 3.0f, static_cast<float>(sampleRate_) * 0.45f), sampleRate_, b0_t, b1_t, b2_t, a1_t, a2_t);
                toneSample = processBiquad(sq * toneEnv, toneFilterL_, b0_t, b1_t, b2_t, a1_t, a2_t) * 1.4f;
                break;
            }
        }

        toneL = toneSample * settings.toneLevel * velocity_;
        toneR = toneL; // Tone centered for solid mono compatibility in club systems
    }

    // 4. Stereo Width (Mid/Side processing without phase cancellation)
    if (settings.noiseStereoWidth != 1.0f) {
        const float mid = (filteredNoiseL + filteredNoiseR) * 0.5f;
        const float side = (filteredNoiseL - filteredNoiseR) * 0.5f * settings.noiseStereoWidth;
        filteredNoiseL = mid + side;
        filteredNoiseR = mid - side;
    }

    // Sum dry clap components
    const float dryL = filteredNoiseL + toneL + clickSample;
    const float dryR = filteredNoiseR + toneR + clickSample;

    // 5. Gated Reverb Tail
    StereoFrame reverbOut = { 0.0f, 0.0f };
    if (settings.reverbMix > 0.001f) {
        reverbOut = reverb_.process(
            dryL, dryR,
            settings.reverbSize,
            settings.reverbDecayMs,
            settings.reverbDamp,
            settings.reverbGateMs,
            sampleRate_);
    }

    const float outL = dryL * (1.0f - settings.reverbMix * 0.5f) + reverbOut.left * settings.reverbMix;
    const float outR = dryR * (1.0f - settings.reverbMix * 0.5f) + reverbOut.right * settings.reverbMix;

    return { outL, outR };
}

void ClapSynth::renderBlock(
    std::vector<float>& leftOut,
    std::vector<float>& rightOut,
    const ClapSynthSettings& settings,
    float velocity,
    int semitoneOffset) noexcept {
    trigger(settings, velocity, semitoneOffset);

    leftOut.resize(totalDurationSamples_);
    rightOut.resize(totalDurationSamples_);

    for (int i = 0; i < totalDurationSamples_; ++i) {
        const auto frame = processSample(settings);
        leftOut[i] = frame.left;
        rightOut[i] = frame.right;
    }
}

} // namespace openclapper::core
