#include "Panels.h"
#include "Parameters.h"
#include "PluginProcessor.h"
#include "Theme.h"

namespace openclapper::ui {

ControlGroup::ControlGroup(Apvts& state, const char* heading, std::initializer_list<ControlSpec> specs,
                           int columns, bool stacked)
    : heading_(heading), columns_(columns), stacked_(stacked) {
    for (const auto& spec : specs) {
        std::unique_ptr<juce::Component> control;
        if (juce::String(spec.id) == "amp_enabled") control = std::make_unique<NoteGate>(state);
        else control = std::make_unique<Control>(state, spec.id, spec.caption);
        addAndMakeVisible(*control);
        controls_.push_back(std::move(control));
    }
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
        g.drawText(heading_, 17, 10, getWidth() - 30, 32, juce::Justification::centredLeft);
        g.setColour(theme::edge);
        g.drawHorizontalLine(49, 17, static_cast<float>(getWidth() - 17));
    } else {
        g.drawFittedText(heading_, 16, 0, 111, getHeight(), juce::Justification::centredLeft, 2);
        g.setColour(theme::edge);
        g.drawVerticalLine(134, 11, static_cast<float>(getHeight() - 11));
    }
}

void ControlGroup::resized() {
    auto area = getLocalBounds();
    if (stacked_) area = area.withTrimmedTop(53).reduced(12, 5);
    else area = area.withTrimmedLeft(143).reduced(5, 4);
    const int rowCount = rows();
    for (int index = 0; index < static_cast<int>(controls_.size()); ++index) {
        const int col = index % columns_, row = index / columns_;
        const int left = area.getX() + col * area.getWidth() / columns_;
        const int top = area.getY() + row * area.getHeight() / rowCount;
        const int right = area.getX() + (col + 1) * area.getWidth() / columns_;
        const int bottom = area.getY() + (row + 1) * area.getHeight() / rowCount;
        controls_[static_cast<size_t>(index)]->setBounds(left, top, right - left, bottom - top);
    }
}

ControlPage::ControlPage(Apvts& state, Page page) : page_(page) {
    using namespace params;
    const auto group = [&](const char* name, int cols, std::initializer_list<ControlSpec> specs) {
        auto item = std::make_unique<ControlGroup>(state, name, specs, cols, page == Page::clap);
        addAndMakeVisible(*item);
        groups_.push_back(std::move(item));
    };
    switch (page) {
        case Page::clap:
            group("Hand spacing", 1, {{kPreBursts,"Bursts"}, {kPreSpread,"Spacing"}, {kPreJitter,"Humanize"}, {kFlamSpread,"Pan spread"}});
            group("Tuned body", 1, {{kToneMode,"Voice"}, {kToneLevel,"Body level"}, {kToneQ,"Resonance"}, {kToneDecay,"Body decay"}});
            group("Noise tail", 1, {{kNoiseType,"Texture"}, {kNoiseHPF,"Low cut"}, {kNoiseDecay,"Noise decay"}, {kNoiseLevel,"Noise level"}});
            group("Finish", 1, {{kClickLevel,"Transient level"}, {kReverbMix,"Room mix"}, {kStereoWidth,"Stereo width"}, {kOutputGain,"Output gain"}});
            break;
        case Page::layers:
            group("Pre-clap", 3, {{kPreBursts,"Bursts"}, {kPreSpread,"Spacing"}, {kPreLevel,"Level"},
                {kPreDecay,"Burst decay"}, {kPreJitter,"Humanize"}, {kFlamSpread,"Pan spread"}});
            group("Tuned body", 4, {{kToneMode,"Voice"}, {kToneQ,"Resonance"}, {kToneDecay,"Decay"}, {kToneLevel,"Level"},
                {kTonePitchEnv,"Pitch sweep"}, {kTonePitchDecay,"Sweep time"}, {kToneHPF,"Low cut"}, {kToneLPF,"High cut"}});
            group("Noise", 5, {{kNoiseType,"Texture"}, {kNoiseAttack,"Attack"}, {kNoiseHold,"Hold"}, {kNoiseDecay,"Decay"}, {kNoiseCurve,"Curve"},
                {kNoiseHPF,"Low cut"}, {kNoiseLPF,"High cut"}, {kNoiseStereoWidth,"Stereo width"}, {kNoiseLevel,"Level"}});
            group("Transient", 4, {{kClickType,"Shape"}, {kClickFreq,"Frequency"}, {kClickDecay,"Decay"}, {kClickLevel,"Level"}});
            break;
        case Page::effects:
            group("Drive", 4, {{kDistType,"Type"}, {kDistDrive,"Drive"}, {kDistTone,"Tone"}, {kDistMix,"Mix"}});
            group("Compressor", 3, {{kCompThresh,"Threshold"}, {kCompRatio,"Ratio"}, {kCompMakeup,"Makeup"},
                {kCompAttack,"Attack"}, {kCompRelease,"Release"}, {kCompMix,"Mix"}});
            group("Gated room", 5, {{kReverbSize,"Size"}, {kReverbDecay,"Decay"}, {kReverbDamp,"Damping"}, {kReverbGate,"Gate time"}, {kReverbMix,"Mix"}});
            group("Echo & reverse", 4, {{kSlapbackDivision,"Slapback time"}, {kSlapbackMix,"Slapback mix"}, {kReverseMode,"Reverse timing"}, {kReverseMix,"Reverse mix"}});
            group("Output", 5, {{kLimiterEnabled,"Limiter"}, {kLimiterCeiling,"Ceiling"}, {kLimiterRelease,"Recovery"}, {kStereoWidth,"Stereo width"}, {kOutputGain,"Output gain"}});
            break;
        case Page::midi:
            group("Note envelope", 5, {{"amp_enabled",""}, {"amp_attack","Attack"}, {"amp_decay","Decay"}, {"amp_sustain","Sustain"}, {"amp_release","Release"}});
            group("Trigger zone", 5, {{"key_low","Lowest note"}, {"key_high","Highest note"}, {"velocity_low","Min velocity"}, {"velocity_high","Max velocity"}, {"choke_group","Choke group"}});
            group("Roll & fill", 2, {{"roll_division","Subdivision"}, {"roll_hits","Hit count"}});
            for (size_t layer = 0; layer < kLayerNames.size(); ++layer) {
                if (layer < kLayerVelocity.size())
                    group(kLayerNames[layer], 3, {{kLayerMute[layer],"Mute"}, {kLayerSolo[layer],"Solo"}, {kLayerVelocity[layer],"Velocity response"}});
                else group(kLayerNames[layer], 3, {{kLayerMute[layer],"Mute"}, {kLayerSolo[layer],"Solo"}});
            }
            break;
    }
}

void ControlPage::resized() {
    auto area = getLocalBounds();
    const int count = static_cast<int>(groups_.size());
    constexpr int gap = 8;
    if (page_ == Page::clap) {
        for (int i = count; i > 0; --i) {
            groups_[static_cast<size_t>(count - i)]->setBounds(area.removeFromLeft((area.getWidth() - (i - 1) * gap) / i));
            area.removeFromLeft(gap);
        }
        return;
    }
    int rows = 0;
    for (const auto& group : groups_) rows += group->rows();
    int available = area.getHeight() - (count - 1) * gap;
    for (const auto& group : groups_) {
        const int h = available * group->rows() / rows;
        group->setBounds(area.removeFromTop(h));
        area.removeFromTop(gap);
        rows -= group->rows();
        available -= h;
    }
}

Waveform::Waveform(OpenClapperProcessor& processor) : processor_(processor) { refresh(); }

void Waveform::refresh() {
    const auto settings = params::readSettings(processor_.parameters).synth;
    if (previous_ && *previous_ == settings) return;
    previous_ = settings;
    strikeMs_ = static_cast<float>(settings.preClapCount) * settings.preClapSpreadMs;
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
