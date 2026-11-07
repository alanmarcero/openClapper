#include "PluginProcessor.h"
#include "Parameters.h"
#include "PluginEditor.h"
#include "Presets.h"

OpenClapperProcessor::OpenClapperProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", openclapper::params::createParameterLayout()) {
}

void OpenClapperProcessor::prepareToPlay(double sampleRate, int) {
    engine_.prepare(sampleRate);
}

bool OpenClapperProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled()) {
        return false;
    }
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
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
    if (auto* playHead = getPlayHead()) {
        if (auto posOpt = playHead->getPosition()) {
            if (posOpt->getBpm()) {
                transport.bpm = *posOpt->getBpm();
                displayBpm.store(static_cast<float>(transport.bpm));
            }
            if (posOpt->getPpqPosition()) {
                transport.ppqPosition = *posOpt->getPpqPosition();
            }
            if (posOpt->getTimeSignature()) {
                transport.beatsPerBar = static_cast<double>(posOpt->getTimeSignature()->numerator);
                displayBeatsPerBar.store(static_cast<float>(transport.beatsPerBar));
            }
            transport.isPlaying = posOpt->getIsPlaying();
        }
    }

    // Parse MIDI triggers & CCs
    std::vector<openclapper::core::MidiTrigger> triggers;
    std::vector<openclapper::core::MidiCcEvent> ccEvents;

    for (const auto meta : midi) {
        const auto msg = meta.getMessage();
        if (msg.isNoteOn()) {
            const float vel = msg.getFloatVelocity();
            displayVelocity.store(vel, std::memory_order_relaxed);
            triggers.push_back({
                .sampleOffset = meta.samplePosition,
                .noteNumber = msg.getNoteNumber(),
                .velocity = vel,
                .audition = false
            });
        } else if (msg.isController()) {
            ccEvents.push_back({
                .sampleOffset = meta.samplePosition,
                .controller = msg.getControllerNumber(),
                .value = msg.getControllerValue()
            });
        }
    }

    if (auditionPending_.exchange(false)) {
        displayVelocity.store(1.0f, std::memory_order_relaxed);
        triggers.push_back({
            .sampleOffset = 0,
            .noteNumber = 69, // A4
            .velocity = 1.0f,
            .audition = true
        });
    }

    // Process through ClapEngine
    const auto settings = openclapper::params::readSettings(parameters);
    float* channelPointers[2] = { buffer.getWritePointer(0), buffer.getWritePointer(1) };

    engine_.process(settings, transport, triggers, channelPointers, 2, numSamples, ccEvents);

    // Update GUI telemetry
    displayPeak.store(engine_.currentPeak());
    displayBeatPhase.store(engine_.currentBeatPhase());
    displayClapProgress.store(engine_.currentClapProgress());
    displayTriggerCount.store(engine_.triggerSerial());
    displayLockedIn.store(engine_.isClapLocked());
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
    const auto& presets = openclapper::presets::factoryPresets();
    if (index >= 0 && index < static_cast<int>(presets.size())) {
        currentProgram_ = index;
        openclapper::presets::apply(presets[static_cast<size_t>(index)], parameters);
        if (auto* param = parameters.getParameter(openclapper::params::kPreset)) {
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(index)));
        }
    }
}

void OpenClapperProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = parameters.copyState();
    if (engine_.isClapLocked()) {
        const auto wav = engine_.wavCache().toWavBytes();
        if (!wav.empty()) {
            state.setProperty("locked_wav", juce::MemoryBlock(wav.data(), wav.size()).toBase64Encoding(), nullptr);
        }
    }
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OpenClapperProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType())) {
        auto state = juce::ValueTree::fromXml(*xmlState);
        parameters.replaceState(state);
        if (state.hasProperty("locked_wav")) {
            juce::MemoryBlock block;
            block.fromBase64Encoding(state.getProperty("locked_wav").toString());
            const auto* bytePtr = static_cast<const uint8_t*>(block.getData());
            std::vector<uint8_t> wavBytes(bytePtr, bytePtr + block.getSize());
            engine_.restoreLockedWav(wavBytes);
            displayLockedIn.store(true);
        }
    }
}

// Juce plugin entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new OpenClapperProcessor();
}
