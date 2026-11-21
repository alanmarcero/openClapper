#pragma once

#include "ClapSynth.h"
#include "Controls.h"
#include <optional>

class OpenClapperProcessor;

namespace openclapper::ui {

struct ControlSpec { const char* id; const char* caption; };

enum class LayerState { audible, muted, notSoloed };

LayerState layerState(const Apvts& state, size_t layer);

// Why a control has no audible effect in the current state, or an empty string when it has one.
juce::String inactiveReason(const Apvts& state, const juce::String& id);

// Where limited peaks land: Ceiling plus Output gain, which runs after the limiter. Empty while the limiter is off.
std::optional<float> outputCeilingDb(const Apvts& state);

class ControlGroup : public juce::Component, private juce::Timer {
public:
    // A card with a layer index gets Mute and Solo chips for that layer.
    ControlGroup(Apvts&, const char* heading, std::initializer_list<ControlSpec>, int columns, bool stacked,
                 bool outputSection = false, int layer = -1);
    void paint(juce::Graphics&) override;
    void resized() override;
    int rows() const;
    // Updates the Lock button, the dimmed controls, the layer status line and the output ceiling text.
    void refresh();
private:
    void timerCallback() override;
    void refreshLockButton();
    Apvts& state_;
    juce::String heading_;
    int columns_;
    bool stacked_;
    bool outputSection_ = false;
    int layer_ = -1;
    juce::String layerStatus_;
    // The Output card frames the limiter and shows where limited peaks land after Output gain.
    juce::Rectangle<int> limiterArea_;
    juce::Label outputCeiling_;
    juce::StringArray paramIds_;
    juce::TextButton lockButton_;
    std::array<juce::TextButton, 2> chips_;
    std::array<std::unique_ptr<Apvts::ButtonAttachment>, 2> chipAttachments_;
    std::vector<std::unique_ptr<juce::Component>> controls_;
};

enum class Page { clap, layers, effects, midi };

class ControlPage : public juce::Component {
public:
    ControlPage(Apvts&, Page);
    void resized() override;
private:
    Page page_;
    std::vector<std::unique_ptr<ControlGroup>> groups_;
};

// A one-shot plot ends where the noise stops or the body has faded. A held note shows 150 ms plus Release.
int waveformDurationMs(const core::ClapSynthSettings& settings);
// "3 bursts, 1.1 ms apart". "(capped)" marks a Spacing that the 5 ms flam limit shortens.
juce::String handsCaption(const core::ClapSynthSettings& settings);

class Waveform : public juce::Component {
public:
    explicit Waveform(OpenClapperProcessor&);
    void refresh();
    void paint(juce::Graphics&) override;
private:
    OpenClapperProcessor& processor_;
    std::optional<core::ClapSynthSettings> previous_;
    std::array<float, 560> upper_{}, lower_{};
    // The first 10 ms of the whole clap, and of the hand bursts alone.
    std::array<float, 160> handsUpper_{}, handsLower_{}, burstsUpper_{}, burstsLower_{};
    juce::String handsCaption_;
    int durationMs_ = 200;
};

// Output peak in dBFS with a 1 s hold, the output ceiling mark, limiter gain reduction and a clip latch.
class OutputMeter : public juce::Component, public juce::SettableTooltipClient {
public:
    OutputMeter();
    // peak and limiterGain are linear extremes since the last call. ceilingDb is empty while the limiter is off.
    void update(float peak, float limiterGain, std::optional<float> ceilingDb, juce::uint32 nowMs);
    [[nodiscard]] bool isClipped() const noexcept { return clipped_; }
    [[nodiscard]] float holdDb() const noexcept { return holdDb_; }
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
private:
    float levelDb_ = -100.0f, holdDb_ = -100.0f, reductionDb_ = 0.0f;
    juce::uint32 lastUpdateMs_ = 0, holdUntilMs_ = 0;
    std::optional<float> ceilingDb_;
    bool clipped_ = false;
};

// Flashes on each hit, and shows an apricot ring when a note falls outside Trigger zone.
class HitLight : public juce::Component, public juce::SettableTooltipClient {
public:
    HitLight(uint32_t hits, uint32_t dropped);
    // A count that moved since the last call starts a flash. Each call fades the flashes.
    void update(uint32_t hits, uint32_t dropped);
    [[nodiscard]] bool isLit() const noexcept { return hitGlow_ > 0.0f || droppedGlow_ > 0.0f; }
    void paint(juce::Graphics&) override;
private:
    uint32_t hits_, dropped_;
    float hitGlow_ = 0.0f, droppedGlow_ = 0.0f;
};

} // namespace openclapper::ui
