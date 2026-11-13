#include "ClapEngine.h"
#include <algorithm>
#include <cmath>

namespace openclapper::core {

namespace {

[[nodiscard]] inline float dbToLin(float db) noexcept {
    return std::pow(10.0f, db / 20.0f);
}

} // namespace

// ============================================================================
// ClapEngine
// ============================================================================

void ClapEngine::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
    for (auto& voice : voices_) {
        voice.synth.prepare(sampleRate);
    }
    slapback_.prepare(sampleRate);
    reverb_.prepare(sampleRate);
    distL_.reset();
    distR_.reset();
    compressor_.prepare(sampleRate);
    limiter_.prepare(sampleRate);

    const int maxReverseSamples = static_cast<int>(sampleRate * 2.0);
    reverseBufferL_.assign(maxReverseSamples, 0.0f);
    reverseBufferR_.assign(maxReverseSamples, 0.0f);
    reset();
}

void ClapEngine::reset() noexcept {
    for (auto& voice : voices_) {
        voice.synth.reset();
        voice.triggerOrder = 0;
    }
    voiceOrderCounter_ = 0;
    lastTriggeredVoice_ = 0;
    slapback_.reset();
    reverb_.reset();
    distL_.reset();
    distR_.reset();
    compressor_.reset();
    limiter_.reset();
    reversePlayHead_ = -1;
    reverseDuration_ = 0;
    isReversing_ = false;
    currentPeak_ = 0.0f;
    currentBeatPhase_ = 0.0f;
    currentClapProgress_ = 1.0f;
    ccVolume_ = 1.0f;
    ccBrightness_ = 1.0f;
}

void ClapEngine::triggerClap(
    const EngineSettings& settings,
    double samplePpq,
    double bpm,
    double beatsPerBar,
    float velocity,
    int midiNote,
    bool reverseCue) noexcept {
    triggerSerial_++;
    currentClapProgress_ = 0.0f;
    reverb_.triggerGate();

    const std::size_t voiceIndex = allocateVoice();
    auto& voice = voices_[voiceIndex];

    // Check if reverse sync should be engaged
    const bool engageReverse = shouldTriggerReverse(settings.effects.reverseMode, reverseCue, samplePpq, beatsPerBar);
    if (engageReverse && settings.effects.reverseMode != ReverseSyncMode::Off) {
        reverseDuration_ = computeSyncedReverseSamples(settings.effects.reverseMode, samplePpq, bpm, beatsPerBar, sampleRate_);
        reverseDuration_ = std::min(reverseDuration_, static_cast<int>(reverseBufferL_.size()));

        // Pre-render a brief clap hit into reverse buffer
        std::vector<float> forwardL, forwardR;
        voice.synth.renderBlock(forwardL, forwardR, settings.synth, velocity, midiNote);

        const int copyCount = std::min(reverseDuration_, static_cast<int>(forwardL.size()));
        for (int i = 0; i < reverseDuration_; ++i) {
            if (i < copyCount) {
                // Reverse envelope swell
                const int srcIdx = copyCount - 1 - i;
                const float ramp = static_cast<float>(i) / static_cast<float>(reverseDuration_);
                reverseBufferL_[i] = forwardL[srcIdx] * ramp * settings.effects.reverseMix;
                reverseBufferR_[i] = forwardR[srcIdx] * ramp * settings.effects.reverseMix;
            } else {
                reverseBufferL_[i] = 0.0f;
                reverseBufferR_[i] = 0.0f;
            }
        }
        reversePlayHead_ = 0;
        isReversing_ = true;
    }

    voice.synth.trigger(settings.synth, midiNote, velocity);
    voice.triggerOrder = ++voiceOrderCounter_;
    lastTriggeredVoice_ = voiceIndex;
}

std::size_t ClapEngine::allocateVoice() const noexcept {
    for (std::size_t i = 0; i < voices_.size(); ++i) {
        if (!voices_[i].synth.isPlaying()) {
            return i;
        }
    }

    std::size_t oldestVoice = 0;
    for (std::size_t i = 1; i < voices_.size(); ++i) {
        if (voices_[i].triggerOrder < voices_[oldestVoice].triggerOrder) {
            oldestVoice = i;
        }
    }
    return oldestVoice;
}

void ClapEngine::process(
    const EngineSettings& settings,
    const Transport& transport,
    std::span<const MidiTrigger> midiTriggers,
    float* const* channels,
    int numChannels,
    int numSamples,
    std::span<const MidiCcEvent> controlChanges) noexcept {
    if (settings.bypass || numChannels < 2 || channels == nullptr || numSamples <= 0) {
        return;
    }

    // Process MIDI Control Changes
    for (const auto& cc : controlChanges) {
        if (cc.controller == 7) {
            ccVolume_ = static_cast<float>(cc.value) / 127.0f;
        } else if (cc.controller == 74) {
            ccBrightness_ = static_cast<float>(cc.value) / 127.0f;
        } else if (cc.controller == 120 || cc.controller == 123) {
            for (auto& voice : voices_) {
                voice.synth.reset();
                voice.triggerOrder = 0;
            }
            voiceOrderCounter_ = 0;
            reversePlayHead_ = -1;
            reverseDuration_ = 0;
            isReversing_ = false;
            slapback_.reset();
            reverb_.reset();
            distL_.reset();
            distR_.reset();
            compressor_.reset();
            limiter_.reset();
        } else if (cc.controller == 121) {
            ccVolume_ = 1.0f;
            ccBrightness_ = 1.0f;
        }
    }

    // Transport position tracking
    double ppq = transport.ppqPosition.value_or(fallbackPpq_);
    const double ppqPerSample = (transport.bpm / 60.0) / sampleRate_;
    const double beatsPerBar = transport.beatsPerBar > 0.0 ? transport.beatsPerBar : 4.0;

    int triggerIdx = 0;
    float blockPeak = 0.0f;
    const float masterGain = dbToLin(settings.effects.outputGainDb) * ccVolume_;

    for (int sample = 0; sample < numSamples; ++sample) {
        // Trigger MIDI events occurring at this sample offset
        bool triggerAtSample = false;
        while (triggerIdx < static_cast<int>(midiTriggers.size()) && midiTriggers[triggerIdx].sampleOffset <= sample) {
            const auto& trig = midiTriggers[triggerIdx++];
            triggerClap(settings, ppq, transport.bpm, beatsPerBar, trig.velocity, trig.noteNumber, trig.audition);
            triggerAtSample = true;
        }

        float sL = 0.0f;
        float sR = 0.0f;
        bool mainHitAtSample = false;

        // Sum all active procedural voices.
        auto activeSettings = settings.synth;
        if (std::abs(ccBrightness_ - 1.0f) > 0.001f) {
            activeSettings.noiseLPF = std::clamp(settings.synth.noiseLPF * ccBrightness_, 200.0f, 20000.0f);
        }
        for (auto& voice : voices_) {
            if (!voice.synth.isPlaying()) {
                continue;
            }
            bool voiceMainHitStarted = false;
            const auto frame = voice.synth.processSample(activeSettings, &voiceMainHitStarted);
            sL += frame.left;
            sR += frame.right;
            mainHitAtSample = mainHitAtSample || voiceMainHitStarted;
        }
        if (mainHitAtSample) {
            reverb_.triggerGate();
        }
        if (voices_[lastTriggeredVoice_].synth.isPlaying()) {
            currentClapProgress_ = voices_[lastTriggeredVoice_].synth.getEnvelopeProgress();
        } else {
            currentClapProgress_ = 1.0f;
        }

        // 2. Mix reverse swell
        if (reversePlayHead_ >= 0 && reversePlayHead_ < reverseDuration_) {
            sL += reverseBufferL_[reversePlayHead_];
            sR += reverseBufferR_[reversePlayHead_];
            reversePlayHead_++;
        } else {
            reversePlayHead_ = -1;
            isReversing_ = false;
        }

        // 3. Shared effects processing
        const bool delayedTrigger = slapback_.process(
            sL,
            sR,
            settings.effects.slapbackDivision,
            settings.effects.slapbackMix,
            transport.bpm,
            triggerAtSample || mainHitAtSample);
        if (delayedTrigger) {
            reverb_.triggerGate();
        }

        const auto reverbFrame = reverb_.process(
            sL,
            sR,
            settings.effects.reverbSize,
            settings.effects.reverbDecayMs,
            settings.effects.reverbDamp,
            settings.effects.reverbGateMs,
            sampleRate_);
        const float reverbMix = std::clamp(settings.effects.reverbMix, 0.0f, 1.0f);
        sL = sL * (1.0f - reverbMix * 0.5f) + reverbFrame.left * reverbMix;
        sR = sR * (1.0f - reverbMix * 0.5f) + reverbFrame.right * reverbMix;

        // Distortion impact shaper
        if (settings.effects.distMix > 0.001f) {
            const float wetDistL = distL_.processSample(sL, settings.effects.distType, settings.effects.distDrive, settings.effects.distTone, sampleRate_);
            const float wetDistR = distR_.processSample(sR, settings.effects.distType, settings.effects.distDrive, settings.effects.distTone, sampleRate_);
            sL = sL * (1.0f - settings.effects.distMix) + wetDistL * settings.effects.distMix;
            sR = sR * (1.0f - settings.effects.distMix) + wetDistR * settings.effects.distMix;
        }

        // Global Stereo Width (Mid/Side processing)
        if (settings.effects.stereoWidth != 1.0f) {
            const float mid = (sL + sR) * 0.5f;
            const float side = (sL - sR) * 0.5f * settings.effects.stereoWidth;
            sL = mid + side;
            sR = mid - side;
        }

        // Dynamics: Compressor
        compressor_.process(sL, sR, settings.effects.compressor);

        // Dynamics: Brickwall Limiter
        limiter_.process(sL, sR, settings.effects.limiter);

        // Apply master gain
        sL *= masterGain;
        sR *= masterGain;

        channels[0][sample] = sL;
        channels[1][sample] = sR;

        const float framePeak = std::max(std::abs(sL), std::abs(sR));
        blockPeak = std::max(blockPeak, framePeak);

        ppq += ppqPerSample;
    }

    fallbackPpq_ = ppq;
    currentPeak_ = blockPeak;
    currentBeatPhase_ = static_cast<float>(std::fmod(ppq, 1.0));
}

} // namespace openclapper::core
