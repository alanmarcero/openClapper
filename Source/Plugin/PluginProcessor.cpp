#include "PluginProcessor.h"
#include "Parameters.h"
#include "PluginEditor.h"
#include "Presets.h"
#include <algorithm>
#include <ranges>

OpenClapperProcessor::OpenClapperProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)
        .withOutput("Body", juce::AudioChannelSet::stereo(), false)
        .withOutput("Noise", juce::AudioChannelSet::stereo(), false)
        .withOutput("Transient", juce::AudioChannelSet::stereo(), false)
        .withOutput("Pre-clap", juce::AudioChannelSet::stereo(), false)),
      parameters(*this, nullptr, "PARAMETERS", openclapper::params::createParameterLayout()) {
    selectPreset(0);
}

void OpenClapperProcessor::prepareToPlay(double sampleRate, int) {
    engine_.prepare(sampleRate);
}

double OpenClapperProcessor::getTailLengthSeconds() const {
    const auto value = [this](const char* id) {
        const auto* parameter = parameters.getRawParameterValue(id);
        return parameter != nullptr ? parameter->load(std::memory_order_relaxed) : 0.0f;
    };

    const double burstCount = value(openclapper::params::kPreBursts);
    const double spreadMs = value(openclapper::params::kPreSpread);
    const double mainStartMs = burstCount * std::max(0.0, spreadMs);
    const double burstTailMs = burstCount > 0.0
        ? mainStartMs + value(openclapper::params::kPreDecay) * 4.0
        : 0.0;
    const double noiseTailMs = value(openclapper::params::kNoiseAttack)
        + value(openclapper::params::kNoiseHold)
        + value(openclapper::params::kNoiseDecay) * 4.0;
    const double mainTailMs = mainStartMs + std::max({
        noiseTailMs,
        value(openclapper::params::kToneDecay) * 6.0,
        20.0
    });
    const double adsrTail = value("amp_enabled") > 0.5f
        ? (value("amp_attack") + value("amp_decay") + value("amp_release")) / 1000.0 : 0.0;
    const double voiceTail = std::max(std::max(burstTailMs, mainTailMs) / 1000.0, adsrTail);

    const bool usesSlapback = value(openclapper::params::kSlapbackDivision) > 0.0f
        && value(openclapper::params::kSlapbackMix) > 0.0f;
    const double delayTail = usesSlapback ? 4.0 * (1.0 + value(openclapper::params::kSlapbackFeedback) * 3.0) : 0.0;
    const double reverseTail = value(openclapper::params::kReverseMode) > 0.0f
        && value(openclapper::params::kReverseMix) > 0.0f
        ? 2.0
        : 0.0;
    const int division = std::clamp(static_cast<int>(value("roll_division")), 0, 4);
    const double reportedBpm = displayBpm.load(std::memory_order_relaxed);
    const double bpm = std::isfinite(reportedBpm) && reportedBpm > 0.0 ? reportedBpm : 138.0;
    const double fillTail = std::max(0.0f, value("roll_hits") - 1.0f) * openclapper::core::kRollDivisionBeats[static_cast<size_t>(division)] * 60.0 / bpm;
    const double effectInputTail = fillTail + std::max(voiceTail, reverseTail) + delayTail;

    if (value(openclapper::params::kReverbMix) <= 0.0f) {
        return effectInputTail;
    }

    const double gateMs = value(openclapper::params::kReverbGate);
    if (gateMs > 0.0) {
        return std::max(effectInputTail, fillTail + delayTail + (gateMs + 18.0) / 1000.0);
    }

    return effectInputTail + value(openclapper::params::kReverbDecay) / 1000.0;
}

bool OpenClapperProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled()) {
        return false;
    }
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) return false;
    for (int bus = 1; bus < layouts.outputBuses.size(); ++bus) {
        const auto set = layouts.outputBuses[bus];
        if (set != juce::AudioChannelSet::disabled() && set != juce::AudioChannelSet::stereo()) return false;
    }
    return true;
}

void OpenClapperProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int ch = 0; ch < numChannels; ++ch) {
        buffer.clear(ch, 0, numSamples);
    }

    if (numChannels < 2) {
        return;
    }

    // Transport acquisition
    openclapper::core::Transport transport;
    if (auto* playHead = getPlayHead(); playHead && wrapperType != wrapperType_Standalone) {
        if (auto posOpt = playHead->getPosition()) {
            if (posOpt->getBpm()) {
                transport.bpm = *posOpt->getBpm();
            }
            if (posOpt->getPpqPosition() && std::isfinite(*posOpt->getPpqPosition())) {
                transport.ppqPosition = *posOpt->getPpqPosition();
            }
            if (posOpt->getTimeSignature()) {
                transport.beatsPerBar = static_cast<double>(posOpt->getTimeSignature()->numerator);
                displayBeatsPerBar.store(static_cast<float>(transport.beatsPerBar));
            }
            transport.isPlaying = posOpt->getIsPlaying();
        }
    }

    const auto settings = openclapper::params::readSettings(parameters);
    if (!std::isfinite(transport.bpm) || transport.bpm <= 0.0) transport.bpm = 138.0;
    displayBpm.store(static_cast<float>(transport.bpm), std::memory_order_relaxed);
    const double beatsPerSample = transport.bpm / (60.0 * getSampleRate());
    const auto eventTransport = [&](int offset) {
        auto position = transport;
        if (position.ppqPosition) *position.ppqPosition += offset * beatsPerSample;
        return position;
    };
    float blockPeak = 0.0f;
    const auto render = [&](int start, int end) {
        if (end <= start) return;
        std::array<float*, 2> main{buffer.getWritePointer(0, start), buffer.getWritePointer(1, start)};
        std::array<float*, 8> components{};
        for (int bus = 1; bus < getBusCount(false); ++bus) {
            if (!getBus(false, bus)->isEnabled()) continue;
            for (int channel = 0; channel < 2; ++channel)
                components[static_cast<size_t>((bus - 1) * 2 + channel)] = buffer.getWritePointer(
                    getChannelIndexInProcessBlockBuffer(false, bus, channel), start);
        }
        engine_.process(settings, eventTransport(start), {}, main.data(), 2, end - start, {}, components);
        blockPeak = std::max(blockPeak, engine_.currentPeak());
    };

    if (auditionPending_.exchange(false)) {
        displayVelocity.store(1.0f, std::memory_order_relaxed);
        engine_.handleMidi(settings, eventTransport(0), {.noteNumber = 69, .velocity = 1.0f, .audition = true});
    }
    // Render between events so arbitrary MIDI density needs no audio-thread container growth.
    int cursor = 0;
    for (const auto meta : midi) {
        const int offset = std::clamp(meta.samplePosition, cursor, numSamples);
        render(cursor, offset);
        cursor = offset;
        if (meta.numBytes > 3) continue;
        const auto message = meta.getMessage();
        if (message.isNoteOnOrOff()) {
            const float velocity = message.getFloatVelocity();
            if (message.isNoteOn()) displayVelocity.store(velocity, std::memory_order_relaxed);
            engine_.handleMidi(settings, eventTransport(offset), {
                .noteNumber = message.getNoteNumber(), .velocity = velocity,
                .noteOff = message.isNoteOff(), .channel = message.getChannel()});
        } else if (message.isController()) {
            const openclapper::core::MidiCcEvent cc{0, message.getControllerNumber(), message.getControllerValue()};
            std::array<float*, 2> main{buffer.getWritePointer(0), buffer.getWritePointer(1)};
            engine_.process(settings, eventTransport(offset), {}, main.data(), 2, 0, std::span(&cc, 1));
        }
    }
    render(cursor, numSamples);

    // Update GUI telemetry
    displayPeak.store(blockPeak);
    const float phase = engine_.currentBeatPhase();
    displayBeatPhase.store(transport.isPlaying ? phase - std::floor(phase) : -1.0f, std::memory_order_relaxed);
    displayClapProgress.store(engine_.currentClapProgress());
    displayTriggerCount.store(engine_.triggerSerial());
}

juce::AudioProcessorEditor* OpenClapperProcessor::createEditor() {
    return new OpenClapperEditor(*this);
}

int OpenClapperProcessor::getNumPrograms() {
    return openclapper::params::kHostProgramCount;
}

const juce::String OpenClapperProcessor::getProgramName(int index) {
    const auto& presets = openclapper::presets::factoryPresets();
    if (index >= 0 && index < static_cast<int>(presets.size())) {
        return presets[static_cast<size_t>(index)].name;
    }
    return {};
}

void OpenClapperProcessor::setCurrentProgram(int index) {
    if (index >= 0 && index < getNumPrograms()) selectPreset(index);
}

void OpenClapperProcessor::selectPreset(int index) {
    const auto& presets = openclapper::presets::factoryPresets();
    if (index < 0 || index >= static_cast<int>(presets.size())) return;
    selectedPreset_ = index;
    openclapper::presets::apply(presets[static_cast<size_t>(index)], parameters, lockedParameterIds());

    // Additional editor presets must not shift either JUCE's host program automation or the legacy selector.
    if (index >= getNumPrograms()) return;
    currentProgram_ = index;
    if (auto* parameter = parameters.getParameter(openclapper::params::kPreset))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(static_cast<float>(index)));
}

bool OpenClapperProcessor::isParamLocked(const juce::String& id) const {
    const juce::ScopedLock guard(lockMutex_);
    return lockedParams_.contains(id);
}

void OpenClapperProcessor::setParamLocked(const juce::String& id, bool locked) {
    {
        const juce::ScopedLock guard(lockMutex_);
        if (locked) lockedParams_.insert(id);
        if (!locked) lockedParams_.erase(id);
    }
    syncLocksToState();
}

bool OpenClapperProcessor::isSectionLocked(const juce::StringArray& ids) const {
    if (ids.isEmpty()) return false;
    const juce::ScopedLock guard(lockMutex_);
    return std::ranges::all_of(ids, [&](const juce::String& id) { return lockedParams_.contains(id); });
}

void OpenClapperProcessor::toggleSectionLock(const juce::StringArray& ids) {
    if (ids.isEmpty()) return;
    {
        const juce::ScopedLock guard(lockMutex_);
        const bool allLocked = std::ranges::all_of(ids, [&](const juce::String& id) { return lockedParams_.contains(id); });
        std::ranges::for_each(ids, [&](const juce::String& id) {
            if (allLocked) {
                lockedParams_.erase(id);
                return;
            }
            lockedParams_.insert(id);
        });
    }
    syncLocksToState();
}

juce::StringArray OpenClapperProcessor::lockedParameterIds() const {
    const juce::ScopedLock guard(lockMutex_);
    juce::StringArray ids;
    std::ranges::for_each(lockedParams_, [&](const juce::String& id) { ids.add(id); });
    return ids;
}

void OpenClapperProcessor::syncLocksToState() {
    const juce::ScopedLock guard(lockMutex_);
    auto tree = parameters.state.getOrCreateChildWithName("ParameterLocks", nullptr);
    tree.removeAllChildren(nullptr);
    std::ranges::for_each(lockedParams_, [&](const juce::String& id) {
        juce::ValueTree child("Lock");
        child.setProperty("id", id, nullptr);
        tree.addChild(child, -1, nullptr);
    });
}

void OpenClapperProcessor::loadLocksFromState(const juce::ValueTree& state) {
    const juce::ScopedLock guard(lockMutex_);
    lockedParams_.clear();
    const auto tree = state.getChildWithName("ParameterLocks");
    if (!tree.isValid()) return;
    std::ranges::for_each(std::views::iota(0, tree.getNumChildren()), [&](int i) {
        const auto id = tree.getChild(i).getProperty("id").toString();
        if (id.isNotEmpty()) lockedParams_.insert(id);
    });
}

void OpenClapperProcessor::getStateInformation(juce::MemoryBlock& destData) {
    syncLocksToState();
    auto state = parameters.copyState();
    state.setProperty("selected_preset", selectedPreset_.load(), nullptr);
    state.setProperty("clap_bank", 2, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OpenClapperProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType())) {
        auto state = juce::ValueTree::fromXml(*xmlState);
        state.removeProperty("locked_wav", nullptr);
        parameters.replaceState(state);
        loadLocksFromState(state);
        currentProgram_ = std::clamp(static_cast<int>(parameters.getRawParameterValue(openclapper::params::kPreset)->load()),
            0, getNumPrograms() - 1);
        selectedPreset_ = static_cast<int>(state.getProperty("clap_bank", 0)) == 2
            ? std::clamp(static_cast<int>(state.getProperty("selected_preset", currentProgram_.load())),
                         -1, static_cast<int>(openclapper::presets::factoryPresets().size()) - 1)
            : -1;
    }
}

// Juce plugin entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new OpenClapperProcessor();
}
