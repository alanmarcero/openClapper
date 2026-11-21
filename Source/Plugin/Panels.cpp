#include "Panels.h"
#include "Parameters.h"
#include "PluginProcessor.h"
#include "Theme.h"
#include <algorithm>
#include <numeric>
#include <ranges>
#include <tuple>

namespace openclapper::ui {

namespace {
struct Dependency {
    const char* id;
    const char* sourceId;
    float activeAbove;
    const char* reason;
};

constexpr const char* kRoomOff = "The room is off while its mix is 0%. Raise the room mix to hear this.";
constexpr const char* kDriveOff = "Drive is off while its mix is 0%. Raise the drive mix to hear this.";
constexpr const char* kCompressorOff = "The compressor is off while its mix is 0%. Raise its mix to hear this.";
constexpr const char* kSlapbackOff = "The slapback is off. Pick a Slapback time to hear this.";
constexpr const char* kReverseOff = "Reverse is off. Pick a Reverse timing to hear this.";
constexpr const char* kOneShot = "One-shot ignores this envelope. Select Hold notes to use it.";
constexpr const char* kRollsOff = "Rolls are off. Pick a Subdivision to hear this.";
constexpr const char* kLimiterOff = "The limiter is off. Turn on Limiter to use this.";
constexpr const char* kOneBurst = "With 1 burst this does almost nothing. Add bursts to hear it.";

// A control is active while its source parameter is above the threshold.
constexpr std::array<Dependency, 24> kDependencies{{
    { params::kReverbSize, params::kReverbMix, 0.0f, kRoomOff },
    { params::kReverbDecay, params::kReverbMix, 0.0f, kRoomOff },
    { params::kReverbDamp, params::kReverbMix, 0.0f, kRoomOff },
    { params::kReverbGate, params::kReverbMix, 0.0f, kRoomOff },
    { params::kDistType, params::kDistMix, 0.0f, kDriveOff },
    { params::kDistDrive, params::kDistMix, 0.0f, kDriveOff },
    { params::kDistTone, params::kDistMix, 0.0f, kDriveOff },
    { params::kCompThresh, params::kCompMix, 0.0f, kCompressorOff },
    { params::kCompRatio, params::kCompMix, 0.0f, kCompressorOff },
    { params::kCompAttack, params::kCompMix, 0.0f, kCompressorOff },
    { params::kCompRelease, params::kCompMix, 0.0f, kCompressorOff },
    { params::kCompMakeup, params::kCompMix, 0.0f, kCompressorOff },
    { params::kSlapbackMix, params::kSlapbackDivision, 0.0f, kSlapbackOff },
    { params::kSlapbackFeedback, params::kSlapbackDivision, 0.0f, kSlapbackOff },
    { params::kReverseMix, params::kReverseMode, 0.0f, kReverseOff },
    { "amp_attack", "amp_enabled", 0.5f, kOneShot },
    { "amp_decay", "amp_enabled", 0.5f, kOneShot },
    { "amp_sustain", "amp_enabled", 0.5f, kOneShot },
    { "amp_release", "amp_enabled", 0.5f, kOneShot },
    { "roll_hits", "roll_division", 0.0f, kRollsOff },
    { params::kLimiterCeiling, params::kLimiterEnabled, 0.5f, kLimiterOff },
    { params::kLimiterRelease, params::kLimiterEnabled, 0.5f, kLimiterOff },
    { params::kPreSpread, params::kPreBursts, 1.0f, kOneBurst },
    { params::kPreJitter, params::kPreBursts, 1.0f, kOneBurst },
}};
} // namespace

juce::String inactiveReason(const Apvts& state, const juce::String& id) {
    const auto dependency = std::ranges::find_if(kDependencies, [&](const Dependency& item) { return id == item.id; });
    if (dependency == kDependencies.end()) return {};
    if (state.getRawParameterValue(dependency->sourceId)->load() > dependency->activeAbove) return {};
    return dependency->reason;
}

ControlGroup::ControlGroup(Apvts& state, const char* heading, std::initializer_list<ControlSpec> specs,
                           int columns, bool stacked, bool outputSection)
    : state_(state), heading_(heading), columns_(columns), stacked_(stacked), outputSection_(outputSection) {
    if (outputSection_) {
        std::ranges::for_each(std::array<std::tuple<const char*, const char*, const char*>, 3>{{
            { params::kEqLowFreq, params::kEqLowDb, "Low EQ" },
            { params::kEqMidFreq, params::kEqMidDb, "Mid EQ" },
            { params::kEqHighFreq, params::kEqHighDb, "High EQ" }
        }}, [&](const auto& eq) {
            const auto& [freqId, dbId, title] = eq;
            paramIds_.add(freqId);
            paramIds_.add(dbId);
            auto knob = std::make_unique<DualEqKnob>(state, freqId, dbId, title);
            addAndMakeVisible(*knob);
            controls_.push_back(std::move(knob));
        });
    }
    std::ranges::for_each(specs, [&](const ControlSpec& spec) {
        paramIds_.add(spec.id);
        std::unique_ptr<juce::Component> control;
        if (juce::String(spec.id) == "amp_enabled") control = std::make_unique<NoteGate>(state);
        if (control == nullptr) control = std::make_unique<Control>(state, spec.id, spec.caption);
        addAndMakeVisible(*control);
        controls_.push_back(std::move(control));
    });

    lockButton_.setComponentID("section_lock_" + heading_);
    lockButton_.setTooltip("Lock or unlock all settings in " + heading_ + " when browsing presets.");
    lockButton_.setColour(juce::TextButton::buttonOnColourId, theme::violet);
    lockButton_.setColour(juce::TextButton::textColourOnId, theme::plot);
    lockButton_.onClick = [this] {
        static_cast<OpenClapperProcessor&>(state_.processor).toggleSectionLock(paramIds_);
        refreshLockButton();
    };
    addAndMakeVisible(lockButton_);
    refresh();
    startTimerHz(10);
}

void ControlGroup::timerCallback() { refresh(); }

void ControlGroup::refresh() {
    refreshLockButton();
    std::ranges::for_each(controls_, [this](const std::unique_ptr<juce::Component>& component) {
        if (auto* control = dynamic_cast<Control*>(component.get()))
            control->setInactiveReason(inactiveReason(state_, control->getComponentID()));
    });
}

void ControlGroup::refreshLockButton() {
    const bool locked = static_cast<OpenClapperProcessor&>(state_.processor).isSectionLocked(paramIds_);
    lockButton_.setToggleState(locked, juce::dontSendNotification);
    lockButton_.setButtonText(locked ? "Locked" : "Lock");
}

int ControlGroup::rows() const { return (static_cast<int>(controls_.size()) + columns_ - 1) / columns_; }

void ControlGroup::paint(juce::Graphics& g) {
    const auto area = getLocalBounds().toFloat();
    g.setColour(theme::surface);
    g.fillRoundedRectangle(area, 8);
    g.setColour(theme::edge);
    g.drawRoundedRectangle(area.reduced(0.5f), 8, 1);
    g.setColour(theme::ink);
    g.setFont(juce::FontOptions(stacked_ ? 22.0f : 17.0f, juce::Font::bold));
    if (stacked_) {
        g.drawText(heading_, 17, 10, getWidth() - 94, 32, juce::Justification::centredLeft);
        g.setColour(theme::edge);
        g.drawHorizontalLine(49, 17, static_cast<float>(getWidth() - 17));
        return;
    }
    g.drawFittedText(heading_, 16, 10, 111, getHeight() - 56, juce::Justification::topLeft, 2);
    g.setColour(theme::edge);
    g.drawVerticalLine(134, 11, static_cast<float>(getHeight() - 11));
}

void ControlGroup::resized() {
    if (stacked_) {
        lockButton_.setBounds(getWidth() - 78, 6, 66, 36);
    }
    if (!stacked_) {
        lockButton_.setBounds(16, std::max(8, getHeight() - 44), 66, 36);
    }
    if (outputSection_) {
        auto area = getLocalBounds().withTrimmedTop(52).reduced(12, 6);
        auto eqArea = area.removeFromLeft(std::min(436, area.getWidth() * 44 / 100));
        const int eqW = eqArea.getWidth() / 3;
        std::ranges::for_each(std::views::iota(0, 3), [&](int i) {
            controls_[static_cast<size_t>(i)]->setBounds(eqArea.removeFromLeft(eqW));
        });
        area.removeFromLeft(8);
        const int gridCols = 3;
        const int otherCount = static_cast<int>(controls_.size()) - 3;
        const int gridRows = std::max(1, (otherCount + gridCols - 1) / gridCols);
        std::ranges::for_each(std::views::iota(0, otherCount), [&](int i) {
            const int col = i % gridCols;
            const int row = i / gridCols;
            const int left = area.getX() + col * area.getWidth() / gridCols;
            const int top = area.getY() + row * area.getHeight() / gridRows;
            const int right = area.getX() + (col + 1) * area.getWidth() / gridCols;
            const int bottom = area.getY() + (row + 1) * area.getHeight() / gridRows;
            controls_[static_cast<size_t>(i + 3)]->setBounds(left, top, right - left, bottom - top);
        });
        return;
    }
    auto area = stacked_ ? getLocalBounds().withTrimmedTop(53).reduced(12, 5)
                         : getLocalBounds().withTrimmedLeft(143).reduced(5, 4);
    const int rowCount = rows();
    std::ranges::for_each(std::views::iota(0, static_cast<int>(controls_.size())), [&](int index) {
        const int col = index % columns_;
        const int row = index / columns_;
        const int left = area.getX() + col * area.getWidth() / columns_;
        const int top = area.getY() + row * area.getHeight() / rowCount;
        const int right = area.getX() + (col + 1) * area.getWidth() / columns_;
        const int bottom = area.getY() + (row + 1) * area.getHeight() / rowCount;
        controls_[static_cast<size_t>(index)]->setBounds(left, top, right - left, bottom - top);
    });
}

ControlPage::ControlPage(Apvts& state, Page page) : page_(page) {
    using namespace params;
    const auto group = [&](const char* name, int cols, std::initializer_list<ControlSpec> specs, bool isOutput = false) {
        auto item = std::make_unique<ControlGroup>(state, name, specs, cols, page == Page::clap || isOutput, isOutput);
        addAndMakeVisible(*item);
        groups_.push_back(std::move(item));
    };
    switch (page) {
        case Page::clap:
            group("Hand spacing", 1, {{kPreBursts,"Bursts"}, {kPreSpread,"Spacing"}, {kPreJitter,"Humanize"}, {kFlamSpread,"Pan spread"}});
            group("Tuned body", 1, {{kToneLevel,"Body level"}, {kToneQ,"Resonance"}, {kToneDecay,"Body decay"}, {kImpactPunch,"Impact"}});
            group("Noise tail", 1, {{kNoiseType,"Texture"}, {kNoiseHPF,"Low cut"}, {kNoiseDecay,"Noise decay"}, {kNoiseLevel,"Noise level"}});
            group("Finish", 1, {{kClickLevel,"Transient level"}, {kReverbMix,"Room mix"}, {kReverbGate,"Room gate"}, {kClapDoubler,"Doubler"}});
            group("Output", 5, {{kStereoWidth,"Stereo width"}, {kOutputGain,"Output gain"}, {kLimiterCeiling,"Ceiling"}, {kLimiterEnabled,"Limiter"}, {kLimiterRelease,"Recovery"}}, true);
            break;
        case Page::layers:
            group("Pre-clap", 3, {{kPreBursts,"Bursts"}, {kPreSpread,"Spacing"}, {kPreLevel,"Level"},
                {kPreDecay,"Burst decay"}, {kPreJitter,"Humanize"}, {kFlamSpread,"Pan spread"}});
            group("Tuned body", 4, {{kToneQ,"Resonance"}, {kToneDecay,"Decay"}, {kToneLevel,"Level"}, {kImpactPunch,"Impact"},
                {kTonePitchEnv,"Pitch sweep"}, {kTonePitchDecay,"Sweep time"}, {kToneHPF,"Low cut"}, {kToneLPF,"High cut"}});
            group("Noise", 3, {{kNoiseType,"Texture"}, {kNoiseAttack,"Attack"}, {kNoiseHold,"Hold"}, {kNoiseDecay,"Decay"}, {kNoiseCurve,"Curve"},
                {kNoiseHPF,"Low cut"}, {kNoiseLPF,"High cut"}, {kNoiseStereoWidth,"Stereo width"}, {kNoiseLevel,"Level"}});
            group("Transient", 4, {{kClickType,"Shape"}, {kClickFreq,"Frequency"}, {kClickDecay,"Decay"}, {kClickLevel,"Level"}});
            break;
        case Page::effects:
            group("Drive", 3, {{kDistType,"Type"}, {kDistDrive,"Drive"}, {kDistTone,"Tone"}, {kDistMix,"Mix"}, {kClapDoubler,"Doubler"}});
            group("Compressor", 3, {{kCompThresh,"Threshold"}, {kCompRatio,"Ratio"}, {kCompMakeup,"Makeup"},
                {kCompAttack,"Attack"}, {kCompRelease,"Release"}, {kCompMix,"Mix"}});
            group("Gated room", 3, {{kReverbSize,"Size"}, {kReverbDecay,"Decay"}, {kReverbDamp,"Damping"}, {kReverbGate,"Gate time"}, {kReverbMix,"Mix"}});
            group("Echo & reverse", 3, {{kSlapbackDivision,"Slapback time"}, {kSlapbackMix,"Slapback mix"}, {kSlapbackFeedback,"Feedback"}, {kReverseMode,"Reverse timing"}, {kReverseMix,"Reverse mix"}});
            break;
        case Page::midi:
            group("Note envelope", 3, {{"amp_enabled",""}, {"amp_attack","Attack"}, {"amp_decay","Decay"}, {"amp_sustain","Sustain"}, {"amp_release","Release"}});
            group("Trigger zone", 3, {{"key_low","Lowest note"}, {"key_high","Highest note"}, {"velocity_low","Min velocity"}, {"velocity_high","Max velocity"}, {"choke_group","Choke group"}});
            group("Roll & fill", 3, {{"roll_division","Subdivision"}, {"roll_hits","Hit count"}, {kPitchHumanize,"Pitch humanize"}});
            std::ranges::for_each(std::views::iota(size_t{0}, kLayerNames.size()), [&](size_t layer) {
                if (layer < kLayerVelocity.size()) {
                    group(kLayerNames[layer], 3, {{kLayerMute[layer],"Mute"}, {kLayerSolo[layer],"Solo"}, {kLayerVelocity[layer],"Velocity response"}});
                    return;
                }
                group(kLayerNames[layer], 3, {{kLayerMute[layer],"Mute"}, {kLayerSolo[layer],"Solo"}});
            });
            break;
    }
}

void ControlPage::resized() {
    auto area = getLocalBounds();
    const int count = static_cast<int>(groups_.size());
    constexpr int gap = 8;
    if (page_ == Page::clap) {
        const int outputH = juce::jlimit(160, 280, getHeight() * 36 / 100);
        auto outputCard = area.removeFromBottom(outputH);
        area.removeFromBottom(gap);
        constexpr int topCount = 4;
        std::ranges::for_each(std::views::iota(0, topCount), [&](int index) {
            const int remaining = topCount - index;
            groups_[static_cast<size_t>(index)]->setBounds(area.removeFromLeft((area.getWidth() - (remaining - 1) * gap) / remaining));
            area.removeFromLeft(gap);
        });
        groups_[4]->setBounds(outputCard);
        return;
    }
    // Every card keeps room for its heading and Lock button. Rows past the first share the rest.
    constexpr int minCardHeight = 80;
    const int rows = std::accumulate(groups_.begin(), groups_.end(), 0,
        [](int total, const std::unique_ptr<ControlGroup>& group) { return total + group->rows(); });
    const int extraRowHeight = (area.getHeight() - (count - 1) * gap - count * minCardHeight) / (rows - count);
    std::ranges::for_each(groups_, [&](const std::unique_ptr<ControlGroup>& group) {
        group->setBounds(area.removeFromTop(minCardHeight + (group->rows() - 1) * extraRowHeight));
        area.removeFromTop(gap);
    });
}

Waveform::Waveform(OpenClapperProcessor& processor) : processor_(processor) { refresh(); }

void Waveform::refresh() {
    const auto settings = params::readSettings(processor_.parameters).synth;
    if (previous_ && *previous_ == settings) return;
    previous_ = settings;
    strikeMs_ = 0.0f;
    durationMs_ = juce::jmax(100, 50 * static_cast<int>(std::ceil(
        (strikeMs_ + std::max(settings.noiseAttackMs + settings.noiseHoldMs + 1.6f * settings.noiseDecayMs,
                              2.5f * settings.toneDecayMs)) / 50.0f)));
    if (settings.ampEnabled) durationMs_ = juce::jlimit(300, 1500, 150 + juce::roundToInt(settings.ampReleaseMs));
    core::ClapSynth voice;
    constexpr int samplesPerMs = 48;
    voice.prepare(samplesPerMs * 1000);
    voice.trigger(settings, 69, 1.0f);
    upper_.fill(0);
    lower_.fill(0);
    const int samples = durationMs_ * samplesPerMs;
    for (int i = 0; i < samples; ++i) {
        if (settings.ampEnabled && i == 150 * samplesPerMs) voice.noteOff(settings);
        const auto frame = voice.processSample(settings);
        const auto bin = static_cast<size_t>(i) * upper_.size() / static_cast<size_t>(samples);
        upper_[bin] = std::max({upper_[bin], frame.left, frame.right});
        lower_[bin] = std::min({lower_[bin], frame.left, frame.right});
    }
    repaint();
}

void Waveform::paint(juce::Graphics& g) {
    const auto area = getLocalBounds();
    g.setColour(theme::plot);
    g.fillRoundedRectangle(area.toFloat(), 10);
    auto inner = area.reduced(23, 12);
    auto caption = inner.removeFromTop(30);
    g.setColour(theme::ink);
    g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    g.drawText("Clap waveform", caption, juce::Justification::centredLeft);
    g.setColour(theme::lilac);
    g.setFont(juce::FontOptions(14.0f));
    g.drawText("Dry signal   /   A4", caption, juce::Justification::centredRight);

    inner.removeFromTop(3);
    const auto ruler = inner.removeFromBottom(19);
    const auto plot = inner.toFloat();
    const float strike = plot.getX() + strikeMs_ / static_cast<float>(durationMs_) * plot.getWidth();
    const float mid = plot.getCentreY() + 4;
    const float maxPeak = std::max(0.12f, std::max(*std::max_element(upper_.begin(), upper_.end()),
                                                    -*std::min_element(lower_.begin(), lower_.end())));
    for (int i = 0; i <= 5; ++i) {
        const int x = juce::roundToInt(plot.getX() + static_cast<float>(i) * plot.getWidth() / 5);
        g.setColour(theme::ink.withAlpha(0.13f));
        g.drawVerticalLine(x, plot.getY(), plot.getBottom());
        g.setColour(theme::lilac);
        g.setFont(juce::FontOptions(12.0f));
        const auto label = juce::String(i * durationMs_ / 5) + " ms";
        g.drawText(label, juce::jlimit(inner.getX(), inner.getRight() - 56, x - 28), ruler.getY(), 56, 19,
            i == 0 ? juce::Justification::centredLeft : i == 5 ? juce::Justification::centredRight : juce::Justification::centred);
    }
    g.setColour(theme::lilac.withAlpha(0.35f));
    g.drawHorizontalLine(juce::roundToInt(mid), plot.getX(), plot.getRight());
    const float step = plot.getWidth() / static_cast<float>(upper_.size());
    const float gain = (plot.getHeight() - 21) * 0.45f / maxPeak;
    for (size_t i = 0; i < upper_.size(); ++i) {
        const float x = plot.getX() + static_cast<float>(i) * step;
        g.setColour(x < strike ? theme::apricot : theme::lilac);
        g.fillRect(x, mid - upper_[i] * gain, step + 0.25f, (upper_[i] - lower_[i]) * gain);
    }
    g.setColour(theme::apricot);
    g.drawVerticalLine(juce::roundToInt(strike), plot.getY() + 15, plot.getBottom());
    g.setFont(juce::FontOptions(12.5f, juce::Font::bold));
    g.drawText("Main strike   " + juce::String(strikeMs_, 1) + " ms", juce::roundToInt(strike + 5),
               inner.getY(), 155, 17, juce::Justification::centredLeft);
}

} // namespace openclapper::ui
