#include "Panels.h"
#include "Parameters.h"
#include "PluginProcessor.h"
#include "Theme.h"
#include <algorithm>
#include <bit>
#include <numeric>
#include <ranges>
#include <span>
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
constexpr const char* kReverseOff = "Reverse is off. Pick a Reverse timing to hear this.";
constexpr const char* kOneShot = "One-shot ignores this envelope. Select Hold notes to use it.";
constexpr const char* kRollsOff = "Rolls are off. Pick a Subdivision to hear this.";
constexpr const char* kLimiterOff = "The limiter is off. Turn on Limiter to use this.";
constexpr const char* kOneBurst = "With 1 burst this does almost nothing. Add bursts to hear it.";
constexpr int kBody = 0, kNoise = 1, kTransient = 2, kPreClap = 3;

// A control is active while its source parameter is above the threshold.
constexpr std::array<Dependency, 22> kDependencies{{
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

struct LayerControl {
    const char* id;
    unsigned layers;
};

constexpr unsigned kBodyBit = 1u << kBody, kNoiseBit = 1u << kNoise, kTransientBit = 1u << kTransient,
                   kPreClapBit = 1u << kPreClap;

// Texture, the noise filters, noise width and Impact shape more than one layer.
constexpr std::array<LayerControl, 31> kLayerControls{{
    { params::kToneQ, kBodyBit }, { params::kToneDecay, kBodyBit }, { params::kTonePitchEnv, kBodyBit },
    { params::kTonePitchDecay, kBodyBit }, { params::kToneHPF, kBodyBit }, { params::kToneLPF, kBodyBit },
    { params::kToneLevel, kBodyBit }, { "body_velocity", kBodyBit }, { params::kImpactPunch, kBodyBit | kNoiseBit },
    { params::kNoiseType, kBodyBit | kNoiseBit | kTransientBit }, { params::kNoiseAttack, kNoiseBit },
    { params::kNoiseHold, kNoiseBit }, { params::kNoiseDecay, kNoiseBit }, { params::kNoiseCurve, kNoiseBit },
    { params::kNoiseResonance, kNoiseBit },
    { params::kNoiseLevel, kNoiseBit }, { "noise_velocity", kNoiseBit }, { params::kNoiseHPF, kNoiseBit | kPreClapBit },
    { params::kNoiseLPF, kNoiseBit | kPreClapBit }, { params::kNoiseStereoWidth, kNoiseBit | kPreClapBit },
    { params::kClickType, kTransientBit }, { params::kClickFreq, kTransientBit }, { params::kClickDecay, kTransientBit },
    { params::kClickLevel, kTransientBit }, { "click_velocity", kTransientBit },
    { params::kPreBursts, kPreClapBit }, { params::kPreSpread, kPreClapBit }, { params::kPreDecay, kPreClapBit },
    { params::kPreJitter, kPreClapBit }, { params::kFlamSpread, kPreClapBit }, { params::kPreLevel, kPreClapBit },
}};
} // namespace

LayerState layerState(const Apvts& state, size_t layer) {
    const auto isOn = [&](const char* id) { return state.getRawParameterValue(id)->load() > 0.5f; };
    if (isOn(params::kLayerMute[layer])) return LayerState::muted;
    if (std::ranges::any_of(params::kLayerSolo, isOn) && !isOn(params::kLayerSolo[layer])) return LayerState::notSoloed;
    return LayerState::audible;
}

juce::String inactiveReason(const Apvts& state, const juce::String& id) {
    const auto dependency = std::ranges::find_if(kDependencies, [&](const Dependency& item) { return id == item.id; });
    if (dependency != kDependencies.end()
        && state.getRawParameterValue(dependency->sourceId)->load() <= dependency->activeAbove)
        return dependency->reason;

    const auto control = std::ranges::find_if(kLayerControls, [&](const LayerControl& item) { return id == item.id; });
    if (control == kLayerControls.end()) return {};
    const auto shapesAudibleLayer = [&](size_t layer) {
        return ((control->layers >> layer) & 1u) != 0 && layerState(state, layer) == LayerState::audible;
    };
    if (std::ranges::any_of(std::views::iota(size_t{ 0 }, params::kLayerNames.size()), shapesAudibleLayer)) return {};
    if (std::popcount(control->layers) > 1) return "Every layer this shapes is silent. Unmute or solo one to hear this.";

    const auto layer = static_cast<size_t>(std::countr_zero(control->layers));
    const juce::String name(params::kLayerNames[layer]);
    if (layerState(state, layer) == LayerState::muted) return "The " + name + " layer is muted. Unmute it to hear this.";
    return "Another layer is soloed. Solo " + name + " too to hear this.";
}

std::optional<float> outputCeilingDb(const Apvts& state) {
    const auto value = [&](const char* id) { return state.getRawParameterValue(id)->load(); };
    if (value(params::kLimiterEnabled) < 0.5f) return std::nullopt;
    return value(params::kLimiterCeiling) + value(params::kOutputGain);
}

ControlGroup::ControlGroup(Apvts& state, const char* heading, std::initializer_list<ControlSpec> specs,
                           int columns, bool stacked, bool outputSection, int layer)
    : state_(state), heading_(heading), columns_(columns), stacked_(stacked), outputSection_(outputSection),
      layer_(layer) {
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
        outputCeiling_.setFont(juce::FontOptions(13.5f, juce::Font::bold));
        outputCeiling_.setColour(juce::Label::textColourId, theme::ink);
        outputCeiling_.setTooltip("Ceiling plus Output gain. Output gain comes after the limiter, so limited peaks land here.");
        addAndMakeVisible(outputCeiling_);
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
    lockButton_.setTitle("Lock " + heading_);
    lockButton_.setTooltip("Lock or unlock all settings in " + heading_ + " when browsing presets.");
    lockButton_.setColour(juce::TextButton::buttonOnColourId, theme::violet);
    lockButton_.setColour(juce::TextButton::textColourOnId, theme::plot);
    lockButton_.onClick = [this] {
        static_cast<OpenClapperProcessor&>(state_.processor).toggleSectionLock(paramIds_);
        refreshLockButton();
    };
    addAndMakeVisible(lockButton_);
    if (layer_ >= 0) {
        const auto index = static_cast<size_t>(layer_);
        const juce::String layerName(params::kLayerNames[index]);
        const std::array<juce::String, 2> actions{ "Mute", "Solo" };
        const std::array<const char*, 2> ids{ params::kLayerMute[index], params::kLayerSolo[index] };
        const std::array<juce::Colour, 2> onColours{ theme::apricot, theme::violet };
        std::ranges::for_each(std::views::iota(size_t{ 0 }, chips_.size()), [&](size_t i) {
            auto& chip = chips_[i];
            chip.setButtonText(actions[i].substring(0, 1));
            chip.setTitle(actions[i] + " " + layerName);
            chip.setTooltip(actions[i] + " the " + layerName + " layer.");
            chip.setClickingTogglesState(true);
            chip.setColour(juce::TextButton::buttonOnColourId, onColours[i]);
            chip.setColour(juce::TextButton::textColourOnId, theme::plot);
            addAndMakeVisible(chip);
            chipAttachments_[i] = std::make_unique<Apvts::ButtonAttachment>(state, ids[i], chip);
        });
    }
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
    if (outputSection_) {
        const auto ceiling = outputCeilingDb(state_);
        const juce::String off("Post-gain ceiling off");
        const auto on = "Post-gain ceiling " + params::formatSigned(ceiling.value_or(0.0f), "dB");
        outputCeiling_.setText(ceiling ? on : off, juce::dontSendNotification);
    }
    if (layer_ < 0) return;

    constexpr std::array<const char*, 3> statusText{ "", "Muted", "Silent: solo on" };
    const juce::String status(statusText[static_cast<size_t>(layerState(state_, static_cast<size_t>(layer_)))]);
    if (status == layerStatus_) return;
    layerStatus_ = status;
    repaint();
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
        g.drawHorizontalLine(layer_ < 0 ? 49 : 88, 17, static_cast<float>(getWidth() - 17));
        if (outputSection_) g.drawRoundedRectangle(limiterArea_.toFloat(), 6, 1);
        if (layer_ < 0) return;
        g.setColour(theme::ink.withAlpha(0.78f));
        g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        g.drawText(juce::String(params::kLayerNames[static_cast<size_t>(layer_)]) + " layer",
                   119, 46, getWidth() - 131, 18, juce::Justification::centredLeft);
        g.setColour(theme::apricot);
        g.drawText(layerStatus_, 119, 64, getWidth() - 131, 18, juce::Justification::centredLeft);
        return;
    }
    g.drawFittedText(heading_, 16, 8, 111, getHeight() - 56, juce::Justification::topLeft, 2);
    g.setColour(theme::edge);
    g.drawVerticalLine(134, 11, static_cast<float>(getHeight() - 11));
    g.setColour(theme::apricot);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText(layerStatus_, 16, 68, 118, 16, juce::Justification::centredLeft);
    if (columns_ == 2 && controls_.size() == 2) {
        const auto bottomRow = getLocalBounds().withTrimmedLeft(143).reduced(5, 4).withTrimmedTop(54);
        g.setColour(theme::secondary);
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.drawText("Fixed velocity", bottomRow.reduced(4, 0), juce::Justification::centredLeft);
    }
}

void ControlGroup::resized() {
    if (stacked_) {
        lockButton_.setBounds(getWidth() - 78, 6, 66, 36);
    }
    if (!stacked_) {
        lockButton_.setBounds(16, std::max(layer_ >= 0 ? 88 : 8, getHeight() - 44), 66, 36);
    }
    if (layer_ >= 0) {
        const int chipsX = stacked_ ? 17 : 16;
        const int chipsY = stacked_ ? 46 : 30;
        chips_[0].setBounds(chipsX, chipsY, 44, 36);
        chips_[1].setBounds(chipsX + 50, chipsY, 44, 36);
    }
    if (outputSection_) {
        auto area = getLocalBounds().withTrimmedTop(52).reduced(12, 6);
        auto eqArea = area.removeFromLeft(std::min(462, area.getWidth() * 46 / 100));
        const int eqW = eqArea.getWidth() / 3;
        std::ranges::for_each(std::views::iota(0, 3), [&](int i) {
            controls_[static_cast<size_t>(i)]->setBounds(eqArea.removeFromLeft(eqW));
        });
        area.removeFromLeft(12);
        const int rowHeight = area.getHeight() / 2;
        auto levels = area.removeFromLeft(190);
        controls_[3]->setBounds(levels.removeFromTop(rowHeight));
        controls_[4]->setBounds(levels);
        area.removeFromLeft(12);
        limiterArea_ = area;
        auto limiter = area.reduced(6, 0);
        auto switchRow = limiter.removeFromTop(rowHeight);
        controls_[5]->setBounds(switchRow.removeFromLeft(160));
        controls_[6]->setBounds(switchRow);
        outputCeiling_.setBounds(limiter.removeFromLeft(160));
        controls_[7]->setBounds(limiter);
        return;
    }
    // The Mute and Solo chips add a second header row to a stacked layer card.
    const int headerHeight = layer_ < 0 ? 53 : 92;
    const int padding = layer_ < 0 ? 5 : 2;
    const auto stackedArea = getLocalBounds().withTrimmedTop(headerHeight).reduced(12, padding);
    const auto sideArea = getLocalBounds().withTrimmedLeft(143).reduced(5, 4);
    const auto area = stacked_ ? stackedArea : sideArea;
    const int totalControls = static_cast<int>(controls_.size());
    const int rowCount = (!stacked_ && columns_ == 2 && totalControls == 2) ? 2 : rows();
    std::ranges::for_each(std::views::iota(0, totalControls), [&](int index) {
        const int col = index % columns_;
        const int row = index / columns_;
        const int itemsInRow = std::min(columns_, totalControls - row * columns_);
        const int left = area.getX() + col * area.getWidth() / itemsInRow;
        const int top = area.getY() + row * area.getHeight() / rowCount;
        const int right = area.getX() + (col + 1) * area.getWidth() / itemsInRow;
        const int bottom = area.getY() + (row + 1) * area.getHeight() / rowCount;
        controls_[static_cast<size_t>(index)]->setBounds(left, top, right - left, bottom - top);
    });
}

ControlPage::ControlPage(Apvts& state, Page page) : page_(page) {
    using namespace params;
    const auto group = [&](const char* name, int cols, std::initializer_list<ControlSpec> specs, bool isOutput = false,
                           int layer = -1) {
        auto item = std::make_unique<ControlGroup>(state, name, specs, cols, page == Page::clap || isOutput, isOutput,
                                                   layer);
        addAndMakeVisible(*item);
        groups_.push_back(std::move(item));
    };
    switch (page) {
        case Page::clap:
            group("Hand spacing", 1, {{kPreBursts,"Bursts"}, {kPreSpread,"Spacing"}, {kPreJitter,"Humanize"}, {kFlamSpread,"Pan spread"}}, false, kPreClap);
            group("Tuned body", 1, {{kToneLevel,"Body level"}, {kToneQ,"Resonance"}, {kToneDecay,"Body decay"}, {kImpactPunch,"Impact"}}, false, kBody);
            group("Noise tail", 1, {{kNoiseType,"Texture"}, {kNoiseHPF,"Low cut"}, {kNoiseDecay,"Noise decay"}, {kNoiseLevel,"Noise level"}}, false, kNoise);
            group("Finish", 1, {{kClickLevel,"Transient level"}, {kReverbMix,"Room mix"}, {kReverbGate,"Room gate"}}, false, kTransient);
            group("Output", 5, {{kStereoWidth,"Stereo width"}, {kOutputGain,"Output gain"}, {kLimiterEnabled,"Limiter"}, {kLimiterCeiling,"Limiter ceiling"}, {kLimiterRelease,"Recovery"}}, true);
            break;
        case Page::layers:
            group("Pre-clap", 3, {{kPreBursts,"Bursts"}, {kPreSpread,"Spacing"}, {kPreLevel,"Level"},
                {kPreDecay,"Burst decay"}, {kPreJitter,"Humanize"}, {kFlamSpread,"Pan spread"}}, false, kPreClap);
            group("Tuned body", 3, {{kToneQ,"Resonance"}, {kToneDecay,"Decay"}, {kToneLevel,"Level"}, {kImpactPunch,"Impact"},
                {kTonePitchEnv,"Pitch sweep"}, {kTonePitchDecay,"Sweep time"}, {kToneHPF,"Low cut"}, {kToneLPF,"High cut"}}, false, kBody);
            group("Noise", 3, {{kNoiseType,"Texture"}, {kNoiseAttack,"Attack"}, {kNoiseHold,"Hold"}, {kNoiseDecay,"Decay"}, {kNoiseCurve,"Curve"},
                {kNoiseHPF,"Low cut"}, {kNoiseLPF,"High cut"}, {kNoiseStereoWidth,"Stereo width"}, {kNoiseLevel,"Level"}, {kNoiseResonance,"Noise tone"}}, false, kNoise);
            group("Transient", 2, {{kClickType,"Shape"}, {kClickFreq,"Frequency"}, {kClickDecay,"Decay"}, {kClickLevel,"Level"}}, false, kTransient);
            break;
        case Page::effects:
            group("Drive", 3, {{kDistType,"Type"}, {kDistDrive,"Drive"}, {kDistTone,"Tone"}, {kDistMix,"Mix"}});
            group("Compressor", 3, {{kCompThresh,"Threshold"}, {kCompRatio,"Ratio"}, {kCompMakeup,"Makeup"},
                {kCompAttack,"Attack"}, {kCompRelease,"Release"}, {kCompMix,"Mix"}});
            group("Gated room", 3, {{kReverbSize,"Size"}, {kReverbDecay,"Decay"}, {kReverbDamp,"Damping"}, {kReverbGate,"Gate time"}, {kReverbMix,"Mix"}});
            group("Echo & reverse", 3, {{kEchoMix,"Echo mix"}, {kReverseMode,"Reverse timing"}, {kReverseMix,"Reverse mix"}});
            break;
        case Page::midi:
            group("Note envelope", 3, {{"amp_enabled",""}, {"amp_attack","Attack"}, {"amp_decay","Decay"}, {"amp_sustain","Sustain"}, {"amp_release","Release"}});
            group("Trigger zone", 3, {{"key_low","Lowest note"}, {"key_high","Highest note"}, {"velocity_low","Min velocity"}, {"velocity_high","Max velocity"}, {"choke_group","Choke group"}});
            group("Roll & fill", 3, {{"roll_division","Subdivision"}, {"roll_hits","Hit count"}, {kKeyTracking,"Key tracking"}});
            std::ranges::for_each(std::views::iota(size_t{0}, kLayerNames.size()), [&](size_t layer) {
                if (layer < kLayerVelocity.size()) {
                    group(kLayerNames[layer], 2, {{kLayerMute[layer],"Mute"}, {kLayerSolo[layer],"Solo"}, {kLayerVelocity[layer],"Velocity response"}});
                    return;
                }
                group(kLayerNames[layer], 2, {{kLayerMute[layer],"Mute"}, {kLayerSolo[layer],"Solo"}});
            });
            break;
    }
}

void ControlPage::resized() {
    auto area = getLocalBounds();
    const int count = static_cast<int>(groups_.size());
    constexpr int gap = 8;
    if (page_ == Page::clap) {
        auto outputCard = area.removeFromBottom(160);
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
    const auto layoutStacked = [&](juce::Rectangle<int> target, size_t start, size_t end) {
        const int n = static_cast<int>(end - start);
        const int rows = std::accumulate(groups_.begin() + static_cast<ptrdiff_t>(start),
                                         groups_.begin() + static_cast<ptrdiff_t>(end), 0,
            [](int total, const std::unique_ptr<ControlGroup>& group) { return total + group->rows(); });
        const int extraRowHeight = (target.getHeight() - (n - 1) * gap - n * minCardHeight) / std::max(1, rows - n);
        for (size_t i = start; i < end; ++i) {
            groups_[i]->setBounds(target.removeFromTop(minCardHeight + (groups_[i]->rows() - 1) * extraRowHeight));
            target.removeFromTop(gap);
        }
    };
    if (page_ == Page::midi && count == 7) {
        constexpr int gridRowHeight = 116;
        auto gridArea = area.removeFromBottom(gridRowHeight * 2 + gap);
        area.removeFromBottom(gap);
        layoutStacked(area, 0, 3);
        const int colWidth = (gridArea.getWidth() - gap) / 2;
        for (int row = 0; row < 2; ++row) {
            auto rowArea = gridArea.removeFromTop(gridRowHeight);
            groups_[static_cast<size_t>(3 + row * 2)]->setBounds(rowArea.removeFromLeft(colWidth));
            rowArea.removeFromLeft(gap);
            groups_[static_cast<size_t>(4 + row * 2)]->setBounds(rowArea);
            gridArea.removeFromTop(gap);
        }
        return;
    }
    layoutStacked(area, 0, groups_.size());
}

namespace {
constexpr int kSamplesPerMs = 48;
constexpr int kHandsMs = 10;
constexpr int kHeldNoteMs = 150;
constexpr float kMinPlotPeak = 0.12f;

void drawBars(juce::Graphics& g, juce::Rectangle<float> plot, std::span<const float> upper,
              std::span<const float> lower, float gain) {
    const float step = plot.getWidth() / static_cast<float>(upper.size());
    std::ranges::for_each(std::views::iota(size_t{ 0 }, upper.size()), [&](size_t i) {
        const float x = plot.getX() + static_cast<float>(i) * step;
        g.fillRect(x, plot.getCentreY() - upper[i] * gain, step + 0.25f, (upper[i] - lower[i]) * gain);
    });
}

void binFrames(std::span<const core::StereoFrame> frames, std::span<float> upper, std::span<float> lower) {
    std::ranges::fill(upper, 0.0f);
    std::ranges::fill(lower, 0.0f);
    std::ranges::for_each(std::views::iota(size_t{ 0 }, frames.size()), [&](size_t i) {
        const auto bin = i * upper.size() / frames.size();
        upper[bin] = std::max({ upper[bin], frames[i].left, frames[i].right });
        lower[bin] = std::min({ lower[bin], frames[i].left, frames[i].right });
    });
}
} // namespace

int waveformDurationMs(const core::ClapSynthSettings& settings) {
    if (settings.ampEnabled) return juce::jlimit(300, 1500, kHeldNoteMs + juce::roundToInt(settings.ampReleaseMs));
    const float noiseEndMs = settings.noiseAttackMs + settings.noiseHoldMs + settings.noiseDecayMs;
    return juce::jmax(100, 50 * static_cast<int>(std::ceil(std::max(noiseEndMs, 2.5f * settings.toneDecayMs) / 50.0f)));
}

juce::String handsCaption(const core::ClapSynthSettings& settings) {
    const int bursts = settings.preClapCount;
    const auto count = juce::String(bursts) + (bursts == 1 ? " burst" : " bursts");
    if (bursts < 2) return count;
    const float spacing = core::flamSpacingMs(bursts, settings.preClapSpreadMs);
    const auto apart = count + ", " + params::formatMilliseconds(spacing) + " apart";
    if (spacing < settings.preClapSpreadMs - 0.01f) return apart + " (capped)";
    return apart;
}

Waveform::Waveform(OpenClapperProcessor& processor) : processor_(processor) { refresh(); }

void Waveform::refresh() {
    const auto settings = params::readSettings(processor_.parameters).synth;
    if (previous_ && *previous_ == settings) return;
    previous_ = settings;
    durationMs_ = waveformDurationMs(settings);
    handsCaption_ = handsCaption(settings);

    core::ClapSynth voice;
    voice.prepare(kSamplesPerMs * 1000);
    voice.trigger(settings, 69, 1.0f);
    std::vector<core::StereoFrame> frames(static_cast<size_t>(durationMs_ * kSamplesPerMs));
    std::vector<core::StereoFrame> bursts(static_cast<size_t>(kHandsMs * kSamplesPerMs));
    std::ranges::for_each(std::views::iota(size_t{ 0 }, frames.size()), [&](size_t i) {
        if (settings.ampEnabled && i == kHeldNoteMs * kSamplesPerMs) voice.noteOff(settings);
        std::array<core::StereoFrame, 4> layers{};
        frames[i] = voice.processSample(settings, nullptr, &layers);
        if (i < bursts.size()) bursts[i] = layers[kPreClap];
    });
    binFrames(frames, upper_, lower_);
    binFrames(std::span(frames).first(bursts.size()), handsUpper_, handsLower_);
    binFrames(bursts, burstsUpper_, burstsLower_);
    repaint();
}

void Waveform::paint(juce::Graphics& g) {
    const auto area = getLocalBounds();
    g.setColour(theme::plot);
    g.fillRoundedRectangle(area.toFloat(), 10);
    auto inner = area.reduced(23, 12);
    auto hands = inner.removeFromRight(250);
    inner.removeFromRight(20);
    auto caption = inner.removeFromTop(30);
    g.setColour(theme::ink);
    g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    g.drawText("Clap waveform", caption, juce::Justification::centredLeft);
    g.setColour(theme::lilac);
    g.setFont(juce::FontOptions(14.0f));
    g.drawText("Dry signal / A4", caption, juce::Justification::centredRight);

    inner.removeFromTop(3);
    const auto ruler = inner.removeFromBottom(19);
    const auto plot = inner.toFloat();
    const float mid = plot.getCentreY();
    const float maxPeak = std::max(kMinPlotPeak, std::max(*std::max_element(upper_.begin(), upper_.end()),
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
    g.setColour(theme::lilac);
    drawBars(g, plot, upper_, lower_, plot.getHeight() * 0.45f / maxPeak);

    // The bursts use their own scale, so a quiet flam stays visible.
    const auto handsTitle = hands.removeFromTop(22);
    g.setColour(theme::ink);
    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    g.drawText("Hands", handsTitle, juce::Justification::centredLeft);
    g.setColour(theme::apricot);
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawText(handsCaption_, handsTitle, juce::Justification::centredRight);
    hands.removeFromTop(4);
    const auto handsRuler = hands.removeFromBottom(19);
    const auto handsPlot = hands.toFloat();
    g.setColour(theme::ink.withAlpha(0.06f));
    g.fillRoundedRectangle(handsPlot, 4);
    const std::array<juce::Justification, 3> rulerJustification{
        juce::Justification::centredLeft, juce::Justification::centred, juce::Justification::centredRight };
    std::ranges::for_each(std::views::iota(0, 3), [&](int i) {
        const int x = juce::roundToInt(handsPlot.getX() + static_cast<float>(i) * handsPlot.getWidth() / 2);
        g.setColour(theme::ink.withAlpha(0.13f));
        g.drawVerticalLine(x, handsPlot.getY(), handsPlot.getBottom());
        g.setColour(theme::lilac);
        g.setFont(juce::FontOptions(12.0f));
        g.drawText(juce::String(i * kHandsMs / 2) + " ms", juce::jlimit(handsRuler.getX(), handsRuler.getRight() - 40, x - 20),
                   handsRuler.getY(), 40, 19, rulerJustification[static_cast<size_t>(i)]);
    });
    const float handsPeak = std::max(kMinPlotPeak, std::max(*std::ranges::max_element(handsUpper_),
                                                            -*std::ranges::min_element(handsLower_)));
    const float burstsPeak = std::max(1.0e-4f, std::max(*std::ranges::max_element(burstsUpper_),
                                                        -*std::ranges::min_element(burstsLower_)));
    g.setColour(theme::lilac.withAlpha(0.3f));
    drawBars(g, handsPlot, handsUpper_, handsLower_, handsPlot.getHeight() * 0.45f / handsPeak);
    g.setColour(theme::apricot);
    drawBars(g, handsPlot, burstsUpper_, burstsLower_, handsPlot.getHeight() * 0.45f / burstsPeak);
    if (previous_->preClapCount > 1) {
        const float spacing = core::flamSpacingMs(previous_->preClapCount, previous_->preClapSpreadMs);
        std::ranges::for_each(std::views::iota(0, previous_->preClapCount), [&](int i) {
            const float x = handsPlot.getX() + static_cast<float>(i) * spacing / kHandsMs * handsPlot.getWidth();
            const float clampedX = std::clamp(x - 1.0f, handsPlot.getX(), handsPlot.getRight() - 2.0f);
            g.fillRect(clampedX, handsPlot.getY(), 2.0f, 8.0f);
        });
    }
}

namespace {
constexpr float kMeterFloorDb = -48.0f;
constexpr float kMeterTopDb = 6.0f;
constexpr float kMeterFallDbPerSecond = 24.0f;
} // namespace

OutputMeter::OutputMeter() {
    setTitle("Output meter");
    setTooltip("Output peak in dBFS with a 1 s hold. The apricot mark is the output ceiling. "
               "GR is limiter gain reduction. Click to clear Clip.");
}

void OutputMeter::update(float peak, float limiterGain, std::optional<float> ceilingDb, juce::uint32 nowMs) {
    // The drawn values in quarter dB. The meter repaints only when one of them moves.
    const auto drawn = [this] {
        const auto quarterDb = [](float db) { return juce::roundToInt(db * 4.0f); };
        return std::array<int, 5>{ quarterDb(levelDb_), quarterDb(holdDb_), quarterDb(reductionDb_),
                                   quarterDb(ceilingDb_.value_or(-100.0f)), clipped_ ? 1 : 0 };
    };
    const auto drawnBefore = drawn();
    const float fallDb = kMeterFallDbPerSecond * static_cast<float>(nowMs - lastUpdateMs_) * 0.001f;
    lastUpdateMs_ = nowMs;
    const float peakDb = juce::Decibels::gainToDecibels(peak, -100.0f);
    levelDb_ = std::max(peakDb, levelDb_ - fallDb);
    reductionDb_ = std::max(-juce::Decibels::gainToDecibels(limiterGain, -100.0f), reductionDb_ - fallDb);
    if (peakDb >= holdDb_ || nowMs >= holdUntilMs_) {
        holdDb_ = levelDb_;
        holdUntilMs_ = nowMs + 1000;
    }
    ceilingDb_ = ceilingDb;
    clipped_ = clipped_ || peak > 1.0f;
    if (drawn() != drawnBefore) repaint();
}

void OutputMeter::mouseDown(const juce::MouseEvent&) {
    clipped_ = false;
    repaint();
}

void OutputMeter::paint(juce::Graphics& g) {
    auto area = getLocalBounds();
    const auto clip = area.removeFromRight(50);
    area.removeFromRight(8);
    const auto labels = area.removeFromLeft(34);
    area.removeFromLeft(4);
    g.setFont(juce::FontOptions(13.5f, juce::Font::bold));
    g.setColour(theme::ink.withAlpha(0.82f));
    g.drawText("Out", labels.withY(area.getY() + 2).withHeight(15), juce::Justification::centredLeft);
    g.drawText("GR", labels.withY(area.getY() + 18).withHeight(15), juce::Justification::centredLeft);

    const auto bars = area.toFloat();
    const auto toX = [&](float db) {
        return bars.getX() + juce::jlimit(0.0f, 1.0f, (db - kMeterFloorDb) / (kMeterTopDb - kMeterFloorDb)) * bars.getWidth();
    };
    const auto peakBar = bars.withY(bars.getY() + 4.0f).withHeight(11.0f);
    const auto reductionBar = bars.withY(bars.getY() + 20.0f).withHeight(11.0f);
    g.setColour(theme::edge);
    g.fillRoundedRectangle(peakBar, 3.0f);
    g.setColour(theme::plot);
    g.fillRoundedRectangle(reductionBar, 3.0f);
    g.setColour(theme::edge);
    g.drawRoundedRectangle(reductionBar.reduced(0.5f), 3.0f, 1.0f);
    g.setColour(theme::violet);
    g.fillRoundedRectangle(peakBar.withRight(toX(levelDb_)), 3.0f);
    g.setColour(theme::secondary);
    g.fillRect(toX(0.0f) - 0.5f, peakBar.getY(), 1.0f, peakBar.getHeight());
    g.setColour(theme::ink);
    if (holdDb_ > kMeterFloorDb) g.fillRect(toX(holdDb_) - 1.0f, peakBar.getY(), 2.0f, peakBar.getHeight());
    g.setColour(theme::apricot);
    if (ceilingDb_) g.fillRect(toX(*ceilingDb_) - 1.0f, peakBar.getY() - 3.0f, 2.0f, peakBar.getHeight() + 6.0f);
    g.fillRoundedRectangle(reductionBar.withWidth(reductionBar.getWidth() * juce::jlimit(0.0f, 1.0f, reductionDb_ / 12.0f)), 3.0f);

    g.setColour(clipped_ ? theme::apricot : theme::edge);
    g.fillRoundedRectangle(clip.toFloat(), 6.0f);
    g.setColour(clipped_ ? theme::plot : theme::secondary);
    g.drawText("Clip", clip, juce::Justification::centred);
}

HitLight::HitLight(uint32_t hits, uint32_t dropped) : hits_(hits), dropped_(dropped) {
    setTitle("Hit light");
    setTooltip("Flashes on each hit. An apricot ring means a note fell outside Trigger zone.");
}

void HitLight::update(uint32_t hits, uint32_t dropped) {
    const bool wasLit = isLit();
    const float hitFade = std::max(0.0f, hitGlow_ - 0.15f);
    const float droppedFade = std::max(0.0f, droppedGlow_ - 0.08f);
    hitGlow_ = hits != hits_ ? 1.0f : hitFade;
    droppedGlow_ = dropped != dropped_ ? 1.0f : droppedFade;
    hits_ = hits;
    dropped_ = dropped;
    if (wasLit || isLit()) repaint();
}

void HitLight::paint(juce::Graphics& g) {
    const auto dot = getLocalBounds().toFloat().withSizeKeepingCentre(18.0f, 18.0f);
    g.setColour(theme::edge);
    g.fillEllipse(dot);
    g.setColour(theme::violet.withAlpha(hitGlow_));
    g.fillEllipse(dot.reduced(2.0f));
    g.setColour(theme::apricot.withAlpha(droppedGlow_));
    g.drawEllipse(dot, 3.0f);
}

} // namespace openclapper::ui
