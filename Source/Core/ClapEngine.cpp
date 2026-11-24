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
    reverseSynth_.prepare(sampleRate);
    reverb_.prepare(sampleRate);
    distL_.reset();
    distR_.reset();
    eq_.prepare(sampleRate);
    compressor_.prepare(sampleRate);
    limiter_.prepare(sampleRate);

    const int maxReverseSamples = static_cast<int>(sampleRate * 2.0);
    reverseBufferL_.assign(maxReverseSamples, 0.0f);
    reverseBufferR_.assign(maxReverseSamples, 0.0f);
    const size_t maxEchoSamples = std::max<size_t>(16, static_cast<size_t>(sampleRate * 2.0));
    echoBufferL_.assign(maxEchoSamples, 0.0f);
    echoBufferR_.assign(maxEchoSamples, 0.0f);
    echoWriteIndex_ = 0;
    reset();
}

void ClapEngine::reset() noexcept {
    for (auto& voice : voices_) {
        voice.synth.reset();
        voice.triggerOrder = 0;
    }
    rolls_ = {};
    fallbackPpq_ = 0.0;
    hasTransportPosition_ = false;
    pendingTrigger_ = false;
    voiceOrderCounter_ = 0;
    lastTriggeredVoice_ = 0;
    reverb_.reset();
    distL_.reset();
    distR_.reset();
    eq_.reset();
    compressor_.reset();
    limiter_.reset();
    std::fill(echoBufferL_.begin(), echoBufferL_.end(), 0.0f);
    std::fill(echoBufferR_.begin(), echoBufferR_.end(), 0.0f);
    echoWriteIndex_ = 0;
    componentDelay_ = {};
    componentDelayPos_ = 0;
    reversePlayHead_ = -1;
    reverseDuration_ = 0;
    isReversing_ = false;
    currentPeak_ = 0.0f;
    currentLimiterGain_ = 1.0f;
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
    pendingTrigger_ = true;
    triggerSerial_++;
    currentClapProgress_ = 0.0f;
    reverb_.triggerGate();

    if (settings.chokeGroup > 0) {
        for (auto& active : voices_)
            if (active.chokeGroup == settings.chokeGroup) active.synth.reset();
    }
    const std::size_t voiceIndex = allocateVoice();
    auto& voice = voices_[voiceIndex];

    // Check if reverse sync should be engaged
    const bool engageReverse = shouldTriggerReverse(settings.effects.reverseMode, reverseCue, samplePpq, beatsPerBar);
    if (engageReverse && settings.effects.reverseMode != ReverseSyncMode::Off) {
        reverseDuration_ = computeSyncedReverseSamples(settings.effects.reverseMode, samplePpq, bpm, beatsPerBar, sampleRate_);
        reverseDuration_ = std::min(reverseDuration_, static_cast<int>(reverseBufferL_.size()));

        // Buffers and the preview voice are prepared before audio processing.
        auto preview = settings.synth;
        preview.ampEnabled = false;
        reverseSynth_.trigger(preview, midiNote, velocity);
        int copyCount = 0;
        while (copyCount < reverseDuration_ && reverseSynth_.isPlaying()) {
            const auto frame = reverseSynth_.processSample(preview);
            if (!reverseSynth_.isPlaying()) break;
            reverseBufferL_[copyCount] = frame.left;
            reverseBufferR_[copyCount++] = frame.right;
        }
        std::reverse(reverseBufferL_.begin(), reverseBufferL_.begin() + copyCount);
        std::reverse(reverseBufferR_.begin(), reverseBufferR_.begin() + copyCount);
        for (int i = 0; i < reverseDuration_; ++i) {
            const float gain = static_cast<float>(i) / static_cast<float>(reverseDuration_) * settings.effects.reverseMix;
            reverseBufferL_[i] = i < copyCount ? reverseBufferL_[i] * gain : 0.0f;
            reverseBufferR_[i] = i < copyCount ? reverseBufferR_[i] * gain : 0.0f;
        }
        reversePlayHead_ = 0;
        isReversing_ = true;
    }

    voice.synth.trigger(settings.synth, midiNote, velocity);
    voice.auditionSamples = reverseCue ? static_cast<int>(sampleRate_ * 0.15) : 0;
    voice.note = midiNote;
    voice.chokeGroup = settings.chokeGroup;
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

void ClapEngine::handleMidi(const EngineSettings& settings, const Transport& transport, const MidiTrigger& trigger) noexcept {
    if (transport.ppqPosition && hasTransportPosition_
        && std::abs(*transport.ppqPosition - fallbackPpq_) > 2.0 / sampleRate_)
        rolls_ = {};
    if (transport.ppqPosition) fallbackPpq_ = *transport.ppqPosition;
    hasTransportPosition_ = transport.ppqPosition.has_value();
    if (trigger.noteOff) {
        for (auto& voice : voices_)
            if (voice.note == trigger.noteNumber && voice.channel == trigger.channel)
                voice.synth.noteOff(settings.synth);
        for (auto& roll : rolls_)
            if (roll.trigger.noteNumber == trigger.noteNumber && roll.trigger.channel == trigger.channel)
                roll.remaining = 0;
        return;
    }

    const int velocity = std::clamp(static_cast<int>(std::lround(trigger.velocity * 127.0f)), 0, 127);
    if (!trigger.audition && (trigger.noteNumber < settings.keyLow || trigger.noteNumber > settings.keyHigh
        || velocity < settings.velocityLow || velocity > settings.velocityHigh)) {
        ++droppedNoteSerial_;
        return;
    }

    const double bpm = std::isfinite(transport.bpm) && transport.bpm > 0.0 ? transport.bpm : 138.0;
    const double ppq = transport.ppqPosition.value_or(fallbackPpq_);
    triggerClap(settings, ppq, bpm, transport.beatsPerBar, trigger.velocity, trigger.noteNumber, trigger.audition);
    voices_[lastTriggeredVoice_].channel = trigger.channel;
    for (auto& roll : rolls_) {
        if ((settings.chokeGroup > 0 && roll.chokeGroup == settings.chokeGroup)
            || (roll.trigger.noteNumber == trigger.noteNumber && roll.trigger.channel == trigger.channel))
            roll.remaining = 0;
    }
    if (settings.rollDivision <= 0 || !transport.isPlaying) return;

    auto* slot = &rolls_.front();
    for (auto& roll : rolls_)
        if (roll.remaining == 0) { slot = &roll; break; }
    const double division = kRollDivisionBeats[static_cast<size_t>(std::clamp(settings.rollDivision, 1, 4))];
    *slot = {trigger, ppq + division, division, std::clamp(settings.rollHits, 1, 32) - 1, settings.chokeGroup};
}

void ClapEngine::process(
    const EngineSettings& settings,
    const Transport& transport,
    std::span<const MidiTrigger> midiTriggers,
    float* const* channels,
    int numChannels,
    int numSamples,
    std::span<const MidiCcEvent> controlChanges,
    std::span<float* const> componentChannels) noexcept {
    if (settings.bypass || numChannels < 2 || channels == nullptr || numSamples < 0) {
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
            rolls_ = {};
            pendingTrigger_ = false;
            voiceOrderCounter_ = 0;
            reversePlayHead_ = -1;
            reverseDuration_ = 0;
            isReversing_ = false;
            std::fill(echoBufferL_.begin(), echoBufferL_.end(), 0.0f);
            std::fill(echoBufferR_.begin(), echoBufferR_.end(), 0.0f);
            echoWriteIndex_ = 0;
            reverb_.reset();
            distL_.reset();
            distR_.reset();
            eq_.reset();
            compressor_.reset();
            limiter_.reset();
        } else if (cc.controller == 121) {
            ccVolume_ = 1.0f;
            ccBrightness_ = 1.0f;
        }
    }

    if (!transport.isPlaying || settings.rollDivision <= 0) rolls_ = {};

    // A seek invalidates pending fills; do not emit their missed hits at the new position.
    if (transport.ppqPosition && hasTransportPosition_
        && std::abs(*transport.ppqPosition - fallbackPpq_) > 2.0 / sampleRate_)
        rolls_ = {};
    hasTransportPosition_ = transport.ppqPosition.has_value();

    // Transport position tracking
    double ppq = transport.ppqPosition.value_or(fallbackPpq_);
    const double bpm = std::isfinite(transport.bpm) && transport.bpm > 0.0 ? transport.bpm : 138.0;
    const double ppqPerSample = (bpm / 60.0) / sampleRate_;
    const double beatsPerBar = transport.beatsPerBar > 0.0 ? transport.beatsPerBar : 4.0;

    int triggerIdx = 0;
    float blockPeak = 0.0f;
    float blockLimiterGain = 1.0f;
    const float masterGain = dbToLin(settings.effects.outputGainDb) * ccVolume_;

    for (int sample = 0; sample < numSamples; ++sample) {
        // Trigger MIDI events occurring at this sample offset
        while (triggerIdx < static_cast<int>(midiTriggers.size()) && midiTriggers[triggerIdx].sampleOffset <= sample) {
            const auto& trig = midiTriggers[triggerIdx++];
            auto eventTransport = transport;
            eventTransport.ppqPosition = ppq;
            fallbackPpq_ = ppq;
            handleMidi(settings, eventTransport, trig);
        }

        for (auto& roll : rolls_) {
            if (roll.remaining > 0 && ppq + ppqPerSample * 0.5 >= roll.nextPpq) {
                triggerClap(settings, ppq, bpm, beatsPerBar, roll.trigger.velocity, roll.trigger.noteNumber, false);
                voices_[lastTriggeredVoice_].channel = roll.trigger.channel;
                if (roll.trigger.audition)
                    voices_[lastTriggeredVoice_].auditionSamples = static_cast<int>(sampleRate_ * 0.15);
                --roll.remaining;
                roll.nextPpq += roll.divisionBeats;
            }
        }

        const bool triggerAtSample = pendingTrigger_;
        pendingTrigger_ = false;
        std::array<StereoFrame, 4> components{};
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
            if (voice.auditionSamples > 0 && --voice.auditionSamples == 0)
                voice.synth.noteOff(activeSettings);
            bool voiceMainHitStarted = false;
            std::array<StereoFrame, 4> voiceComponents{};
            const auto frame = voice.synth.processSample(activeSettings, &voiceMainHitStarted, &voiceComponents);
            for (size_t layer = 0; layer < components.size(); ++layer) {
                components[layer].left += voiceComponents[layer].left;
                components[layer].right += voiceComponents[layer].right;
            }
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

        // 3. Shared effects processing. Dotted-eighth echo, same as the hat and the snare.
        const float echoMix = std::clamp(settings.effects.echoMix, 0.0f, 1.0f);
        if (!echoBufferL_.empty() && echoMix > 0.001f) {
            const size_t bufSize = echoBufferL_.size();
            const double safeBpm = std::clamp(std::isfinite(bpm) && bpm > 0.0 ? bpm : 138.0, 40.0, 260.0);
            const size_t delaySamples = std::clamp<size_t>(
                static_cast<size_t>(0.375 * 60.0 / safeBpm * sampleRate_), 1, bufSize - 1);
            const size_t readIdx = (echoWriteIndex_ + bufSize - delaySamples) % bufSize;
            const float delayedL = echoBufferL_[readIdx];
            const float delayedR = echoBufferR_[readIdx];
            echoBufferL_[echoWriteIndex_] = std::tanh(sL + delayedR * 0.35f);
            echoBufferR_[echoWriteIndex_] = std::tanh(sR + delayedL * 0.35f);
            echoWriteIndex_ = (echoWriteIndex_ + 1) % bufSize;
            sL += delayedL * echoMix * 0.65f;
            sR += delayedR * echoMix * 0.65f;
        }

        const float reverbMix = std::clamp(settings.effects.reverbMix, 0.0f, 1.0f);
        if (reverbMix > 0.001f) {
            const auto reverbFrame = reverb_.process(
                sL,
                sR,
                settings.effects.reverbSize,
                settings.effects.reverbDecayMs,
                settings.effects.reverbDamp,
                settings.effects.reverbGateMs,
                sampleRate_);
            sL = sL * (1.0f - reverbMix * 0.5f) + reverbFrame.left * reverbMix;
            sR = sR * (1.0f - reverbMix * 0.5f) + reverbFrame.right * reverbMix;
        }

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

        eq_.process(sL, sR, settings.effects.eq);

        // Dynamics: Compressor
        compressor_.process(sL, sR, settings.effects.compressor);

        // Dynamics: Brickwall Limiter
        limiter_.process(sL, sR, settings.effects.limiter);
        blockLimiterGain = std::min(blockLimiterGain, limiter_.gain());

        // Apply master gain
        sL *= masterGain;
        sR *= masterGain;

        const auto delayedComponents = componentDelay_[componentDelayPos_];
        componentDelay_[componentDelayPos_] = components;
        componentDelayPos_ = (componentDelayPos_ + 1) % componentDelay_.size();

        for (size_t channel = 0; channel < componentChannels.size() && channel < 8; ++channel) {
            if (componentChannels[channel] == nullptr) continue;
            const auto& frame = delayedComponents[channel / 2];
            componentChannels[channel][sample] = (channel % 2 == 0 ? frame.left : frame.right) * masterGain;
        }
        channels[0][sample] = sL;
        channels[1][sample] = sR;

        const float framePeak = std::max(std::abs(sL), std::abs(sR));
        blockPeak = std::max(blockPeak, framePeak);

        ppq += ppqPerSample;
    }

    fallbackPpq_ = ppq;
    currentPeak_ = blockPeak;
    currentLimiterGain_ = blockLimiterGain;
    currentBeatPhase_ = static_cast<float>(std::fmod(ppq, 1.0));
}

} // namespace openclapper::core
