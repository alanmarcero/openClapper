#include "PluginProcessor.h"
#include "Parameters.h"
#include "PluginEditor.h"
#include "Presets.h"
#include <algorithm>
#include <cmath>
#include <juce_audio_formats/juce_audio_formats.h>
#include <ranges>

OpenClapperProcessor::OpenClapperProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)
        .withOutput("Body", juce::AudioChannelSet::stereo(), false)
        .withOutput("Noise", juce::AudioChannelSet::stereo(), false)
        .withOutput("Transient", juce::AudioChannelSet::stereo(), false)
        .withOutput("Pre-clap", juce::AudioChannelSet::stereo(), false)),
      parameters(*this, nullptr, "PARAMETERS", openclapper::params::createParameterLayout()) {
    undo_.setStateAccess(
        [this] { return captureSoundState(); },
        [this](const juce::ValueTree& tree) { applySoundTree(tree, ApplyKind::undo); });
    undo_.attach(*this);
    undo_.suspend();
    selectPreset(0);
    undo_.resume();
    undo_.clear();
    ab_.setBoth(captureSoundState());
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
        return std::max(effectInputTail, fillTail + delayTail + (gateMs + openclapper::core::kReverbGateReleaseMs) / 1000.0);
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
    return static_cast<int>(openclapper::presets::factoryPresets().size());
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
    undo_.group("Preset", [&] {
        selectedPreset_ = index;
        setUserPresetName({});
        openclapper::presets::apply(presets[static_cast<size_t>(index)], parameters, lockedParameterIds());
        currentProgram_ = index;
        // Preset Index automation stays on the original 44 positions.
        if (index < openclapper::params::kHostProgramCount)
            if (auto* parameter = parameters.getParameter(openclapper::params::kPreset))
                parameter->setValueNotifyingHost(parameter->convertTo0to1(static_cast<float>(index)));
    });
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

juce::ValueTree OpenClapperProcessor::captureSoundState() {
    syncLocksToState();
    auto state = parameters.copyState();
    state.removeChild(state.getChildWithName("ABCompare"), nullptr);
    state.removeProperty("editor_width", nullptr);
    state.removeProperty("editor_height", nullptr);
    state.setProperty("selected_preset", selectedPreset_.load(), nullptr);
    state.setProperty("host_program", currentProgram_.load(), nullptr);
    state.setProperty("user_preset", userPresetName(), nullptr);
    state.setProperty("clap_bank", 3, nullptr);
    return state;
}

void OpenClapperProcessor::applyParameterChildren(const juce::ValueTree& tree, const juce::StringArray& lockedIds) {
    for (int index = 0; index < tree.getNumChildren(); ++index) {
        const auto child = tree.getChild(index);
        if (!child.hasProperty("id") || !child.hasProperty("value")) continue;
        const auto id = child.getProperty("id").toString();
        if (lockedIds.contains(id)) continue;
        if (auto* parameter = parameters.getParameter(id)) {
            const float value = static_cast<float>(child.getProperty("value"));
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        }
    }
}

void OpenClapperProcessor::applySoundTree(const juce::ValueTree& incoming, ApplyKind kind) {
    undo_.suspend();
    auto state = incoming.createCopy();
    state.removeChild(state.getChildWithName("ABCompare"), nullptr);
    state.removeProperty("locked_wav", nullptr);
    const int width = static_cast<int>(state.getProperty("editor_width", editorWidth_.load()));
    const int height = static_cast<int>(state.getProperty("editor_height", editorHeight_.load()));
    state.removeProperty("editor_width", nullptr);
    state.removeProperty("editor_height", nullptr);
    parameters.replaceState(state);

    const int bank = static_cast<int>(state.getProperty("clap_bank", 0));
    if (bank < 3 && !state.getChildWithProperty("id", openclapper::params::kImpactPunch).isValid()) {
        if (auto* impactParam = parameters.getParameter(openclapper::params::kImpactPunch))
            impactParam->setValueNotifyingHost(impactParam->convertTo0to1(0.0f));
    }
    loadLocksFromState(state);
    const auto clampDelay = [this](const char* id) {
        auto* parameter = parameters.getParameter(id);
        const auto* raw = parameters.getRawParameterValue(id);
        if (parameter == nullptr || raw == nullptr) return;
        if (raw->load() > openclapper::core::kMaxTimingOffsetMs)
            parameter->setValueNotifyingHost(parameter->convertTo0to1(openclapper::core::kMaxTimingOffsetMs));
    };
    clampDelay(openclapper::params::kPreSpread);
    clampDelay(openclapper::params::kNoiseAttack);

    const int factoryCount = static_cast<int>(openclapper::presets::factoryPresets().size());
    const int legacyProgram = std::clamp(
        static_cast<int>(parameters.getRawParameterValue(openclapper::params::kPreset)->load()),
        0, openclapper::params::kHostProgramCount - 1);
    currentProgram_ = state.hasProperty("host_program")
        ? std::clamp(static_cast<int>(state.getProperty("host_program")), 0, factoryCount - 1)
        : legacyProgram;
    selectedPreset_ = bank == 3
        ? std::clamp(static_cast<int>(state.getProperty("selected_preset", currentProgram_.load())), -1, factoryCount - 1)
        : -1;
    setUserPresetName(state.getProperty("user_preset").toString());

    if (kind == ApplyKind::session) {
        if (incoming.hasProperty("editor_width") && incoming.hasProperty("editor_height")) {
            editorWidth_ = std::clamp(width, 100, 8000);
            editorHeight_ = std::clamp(height, 100, 8000);
        }
        if (!ab_.read(incoming)) ab_.setBoth(captureSoundState());
    }
    undo_.resume();
    if (kind != ApplyKind::undo) undo_.clear();
}

void OpenClapperProcessor::getStateInformation(juce::MemoryBlock& destData) {
    if (ab_.hasSlots()) ab_.store(ab_.editingB(), captureSoundState());
    auto state = captureSoundState();
    if (editorWidth_.load() > 0) state.setProperty("editor_width", editorWidth_.load(), nullptr);
    if (editorHeight_.load() > 0) state.setProperty("editor_height", editorHeight_.load(), nullptr);
    ab_.write(state);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OpenClapperProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        applySoundTree(juce::ValueTree::fromXml(*xmlState), ApplyKind::session);
}

void OpenClapperProcessor::setUserPresetName(juce::String name) {
    const juce::ScopedLock guard(lockMutex_);
    userPresetName_ = std::move(name);
}

juce::String OpenClapperProcessor::userPresetName() const {
    const juce::ScopedLock guard(lockMutex_);
    return userPresetName_;
}

void OpenClapperProcessor::setEditorSize(int width, int height) noexcept {
    if (width > 0 && height > 0) {
        editorWidth_ = width;
        editorHeight_ = height;
    }
}

void OpenClapperProcessor::toggleAb() {
    if (!ab_.hasSlots()) ab_.setBoth(captureSoundState());
    undo_.suspend();
    ab_.store(ab_.editingB(), captureSoundState());
    ab_.setEditingB(!ab_.editingB());
    applySoundTree(ab_.slot(ab_.editingB()), ApplyKind::slot);
    undo_.resume();
    undo_.clear();
}

void OpenClapperProcessor::copyAbAToB() {
    if (!ab_.hasSlots()) ab_.setBoth(captureSoundState());
    undo_.suspend();
    ab_.store(ab_.editingB(), captureSoundState());
    ab_.copyAToB();
    if (ab_.editingB()) applySoundTree(ab_.slot(true), ApplyKind::slot);
    undo_.resume();
    undo_.clear();
}

void OpenClapperProcessor::copyAbBToA() {
    if (!ab_.hasSlots()) ab_.setBoth(captureSoundState());
    undo_.suspend();
    ab_.store(ab_.editingB(), captureSoundState());
    ab_.copyBToA();
    if (!ab_.editingB()) applySoundTree(ab_.slot(false), ApplyKind::slot);
    undo_.resume();
    undo_.clear();
}

void OpenClapperProcessor::loadUserPreset(const juce::String& name) {
    auto tree = userPresets_.load(name);
    if (!tree) return;
    undo_.group("User preset", [&] {
        applyParameterChildren(*tree, lockedParameterIds());
        selectedPreset_ = -1;
        setUserPresetName(suite::UserPresetStore::sanitise(name));
    });
}

bool OpenClapperProcessor::saveUserPreset(const juce::String& name) {
    const auto clean = suite::UserPresetStore::sanitise(name);
    if (clean.isEmpty()) return false;
    if (!userPresets_.save(clean, captureSoundState())) return false;
    setUserPresetName(clean);
    selectedPreset_ = -1;
    return true;
}

bool OpenClapperProcessor::renameUserPreset(const juce::String& from, const juce::String& to) {
    if (!userPresets_.rename(from, to)) return false;
    if (userPresetName() == suite::UserPresetStore::sanitise(from))
        setUserPresetName(suite::UserPresetStore::sanitise(to));
    return true;
}

bool OpenClapperProcessor::deleteUserPreset(const juce::String& name) {
    if (!userPresets_.remove(name)) return false;
    if (userPresetName() == suite::UserPresetStore::sanitise(name)) setUserPresetName({});
    return true;
}

juce::File OpenClapperProcessor::renderHitWav(const juce::File& destination) const {
    if (destination == juce::File()) return {};
    destination.getParentDirectory().createDirectory();
    constexpr double sampleRate = 48000.0;
    const int samples = std::clamp(
        static_cast<int>(std::ceil((getTailLengthSeconds() + 0.2) * sampleRate)),
        static_cast<int>(sampleRate * 0.4),
        static_cast<int>(sampleRate * 12.0));

    openclapper::core::ClapEngine engine;
    engine.prepare(sampleRate);
    const auto settings = openclapper::params::readSettings(parameters);
    openclapper::core::Transport transport;
    transport.bpm = 138.0;
    transport.ppqPosition = 0.0;
    transport.beatsPerBar = 4.0;
    transport.isPlaying = false;
    const openclapper::core::MidiTrigger trigger{.noteNumber = 69, .velocity = 1.0f, .audition = true};

    juce::AudioBuffer<float> buffer(2, samples);
    buffer.clear();
    int rendered = 0;
    while (rendered < samples) {
        const int block = std::min(512, samples - rendered);
        float* channels[] = {buffer.getWritePointer(0, rendered), buffer.getWritePointer(1, rendered)};
        if (rendered == 0)
            engine.process(settings, transport, std::span(&trigger, 1), channels, 2, block);
        else
            engine.process(settings, transport, {}, channels, 2, block);
        rendered += block;
    }

    const int fade = std::min(samples, static_cast<int>(sampleRate * 0.01));
    for (int index = 0; index < fade; ++index) {
        const float gain = static_cast<float>(fade - index) / static_cast<float>(fade);
        buffer.applyGain(samples - fade + index, 1, gain);
    }

    juce::WavAudioFormat format;
    std::unique_ptr<juce::OutputStream> stream(destination.createOutputStream());
    if (stream == nullptr) return {};
    auto writer = format.createWriterFor(stream, juce::AudioFormatWriterOptions{}
        .withSampleRate(sampleRate)
        .withNumChannels(2)
        .withBitsPerSample(24));
    if (writer == nullptr || !writer->writeFromAudioSampleBuffer(buffer, 0, samples)) return {};
    return destination;
}

// Juce plugin entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new OpenClapperProcessor();
}
