#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <utility>
#include <vector>

namespace openclapper::presets {

// Clap, hat, and kick banks are 56 programs. The snare bank is 84.
inline constexpr int kFactoryProgramCount = 56;

struct FactoryPreset {
    const char* name;
    std::vector<std::pair<juce::String, float>> values;
};

const std::vector<FactoryPreset>& factoryPresets();
void apply(const FactoryPreset& preset, juce::AudioProcessorValueTreeState& state,
           const juce::StringArray& lockedIds = {});

} // namespace openclapper::presets
