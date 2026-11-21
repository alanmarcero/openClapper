#include "Controls.h"
#include "PluginProcessor.h"
#include "Theme.h"

#include <algorithm>
#include <cmath>

namespace openclapper::ui {

namespace {
// A drag moves the value from where it was, so a click never jumps it. Shift-drag is 10 times finer.
class FineSlider final : public juce::Slider {
public:
    FineSlider() { setSliderSnapsToMousePosition(false); }

    void mouseDown(const juce::MouseEvent& event) override {
        const int track = std::max(1, getLookAndFeel().getSliderLayout(*this).sliderBounds.getWidth());
        setMouseDragSensitivity(track * (event.mods.isShiftDown() ? 10 : 1));
        juce::Slider::mouseDown(event);
    }
};
} // namespace

LockBadge::LockBadge(Apvts& state, const juce::String& parameterId)
    : state_(state), id_(parameterId) {
    setComponentID("lock_" + id_);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    setTooltip("Lock or unlock this setting for factory presets.");
    lastLocked_ = static_cast<OpenClapperProcessor&>(state_.processor).isParamLocked(id_);
    startTimerHz(10);
}

void LockBadge::timerCallback() {
    const bool locked = static_cast<OpenClapperProcessor&>(state_.processor).isParamLocked(id_);
    if (locked != lastLocked_) {
        lastLocked_ = locked;
        repaint();
    }
}

void LockBadge::paint(juce::Graphics& g) {
    const bool locked = static_cast<OpenClapperProcessor&>(state_.processor).isParamLocked(id_);
    lastLocked_ = locked;
    const auto cx = static_cast<float>(getWidth()) * 0.5f;
    const auto cy = static_cast<float>(getHeight()) * 0.5f;
    const auto colour = locked ? theme::violet : theme::secondary;
    g.setColour(colour.withAlpha(locked || isMouseOver() ? 1.0f : 0.6f));
    juce::Path shackle;
    shackle.startNewSubPath(cx - 3.5f, cy - 1.0f);
    shackle.lineTo(cx - 3.5f, cy - 4.0f);
    shackle.cubicTo(cx - 3.5f, cy - 8.5f, cx + 3.5f, cy - 8.5f, cx + 3.5f, cy - 4.0f);
    if (locked) shackle.lineTo(cx + 3.5f, cy - 1.0f);
    g.strokePath(shackle, juce::PathStrokeType(1.5f));
    g.drawRoundedRectangle(cx - 5.5f, cy - 1.5f, 11.0f, 8.5f, 1.8f, 1.5f);
}

void LockBadge::mouseDown(const juce::MouseEvent&) {
    auto& proc = static_cast<OpenClapperProcessor&>(state_.processor);
    proc.setParamLocked(id_, !proc.isParamLocked(id_));
    lastLocked_ = proc.isParamLocked(id_);
    repaint();
}

Control::Control(Apvts& state, const char* id, const char* caption)
    : lock_(std::make_unique<LockBadge>(state, id)) {
    setComponentID(id);
    addAndMakeVisible(*lock_);
    auto* parameter = state.getParameter(id);
    jassert(parameter != nullptr);
    if (dynamic_cast<juce::AudioParameterBool*>(parameter) != nullptr) {
        toggle_ = std::make_unique<juce::TextButton>();
        toggle_->setButtonText(caption);
        toggle_->setName(parameter->getName(80));
        toggle_->setTooltip(parameter->getName(80));
        toggle_->setClickingTogglesState(true);
        addAndMakeVisible(*toggle_);
        toggleAttachment_ = std::make_unique<Apvts::ButtonAttachment>(state, id, *toggle_);
        return;
    }

    label_.setText(caption, juce::dontSendNotification);
    label_.setFont(juce::FontOptions(14.5f, juce::Font::bold));
    label_.setBorderSize({});
    label_.setColour(juce::Label::textColourId, theme::ink);
    addAndMakeVisible(label_);
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(parameter)) {
        choice_ = std::make_unique<juce::ComboBox>();
        choice_->setName(parameter->getName(80));
        choice_->setTooltip(parameter->getName(80));
        choice_->addItemList(choice->choices, 1);
        addAndMakeVisible(*choice_);
        choiceAttachment_ = std::make_unique<Apvts::ComboBoxAttachment>(state, id, *choice_);
        return;
    }

    slider_ = std::make_unique<FineSlider>();
    slider_->setName(parameter->getName(80));
    slider_->setSliderStyle(juce::Slider::LinearHorizontal);
    slider_->setColour(juce::Slider::textBoxTextColourId, theme::ink);
    slider_->setColour(juce::Slider::textBoxBackgroundColourId, theme::paper);
    slider_->setColour(juce::Slider::textBoxOutlineColourId, theme::edge);
    slider_->setTextBoxStyle(juce::Slider::TextBoxRight, false, 68, 22);
    addAndMakeVisible(*slider_);
    sliderAttachment_ = std::make_unique<Apvts::SliderAttachment>(state, id, *slider_);
}

void Control::setInactiveReason(const juce::String& reason) {
    setAlpha(reason.isEmpty() ? 1.0f : 0.4f);
    label_.setTooltip(reason);
    if (slider_ != nullptr) slider_->setTooltip(reason);
    if (choice_ == nullptr) return;
    if (reason.isEmpty()) {
        choice_->setTooltip(choice_->getName());
        return;
    }
    choice_->setTooltip(reason);
}

void Control::resized() {
    auto area = getLocalBounds().reduced(4, 1);
    lock_->setBounds(area.removeFromRight(24).withSizeKeepingCentre(22, 22));
    area.removeFromRight(2);
    if (toggleAttachment_) {
        toggle_->setBounds(area.withSizeKeepingCentre(juce::jmin(144, area.getWidth()), juce::jmin(36, area.getHeight())));
        return;
    }
    const int contentHeight = choiceAttachment_ ? 54 : 44;
    area = area.withSizeKeepingCentre(area.getWidth(), juce::jmin(contentHeight, area.getHeight()));
    label_.setBounds(area.removeFromTop(18));
    if (choiceAttachment_) {
        choice_->setBounds(area);
        return;
    }
    slider_->setBounds(area);
}

NoteGate::NoteGate(Apvts& state)
    : attachment_(*state.getParameter("amp_enabled"), [this](float value) {
        oneShot_.setToggleState(value < 0.5f, juce::dontSendNotification);
        held_.setToggleState(value >= 0.5f, juce::dontSendNotification);
      }),
      lock_(state, "amp_enabled") {
    setComponentID("amp_enabled");
    oneShot_.setTooltip("Play each layer through its one-shot decay.");
    held_.setTooltip("Sustain the body and noise until a matching MIDI note-off.");
    oneShot_.onClick = [this] { attachment_.setValueAsCompleteGesture(0.0f); };
    held_.onClick = [this] { attachment_.setValueAsCompleteGesture(1.0f); };
    addAndMakeVisible(oneShot_);
    addAndMakeVisible(held_);
    addAndMakeVisible(lock_);
    attachment_.sendInitialUpdate();
}

void NoteGate::resized() {
    auto bounds = getLocalBounds().reduced(4, 0);
    lock_.setBounds(bounds.removeFromRight(24).withSizeKeepingCentre(22, 22));
    bounds.removeFromRight(2);
    bounds = bounds.withSizeKeepingCentre(bounds.getWidth(), 36);
    oneShot_.setBounds(bounds.removeFromLeft(bounds.getWidth() / 2).withTrimmedRight(2));
    held_.setBounds(bounds.withTrimmedLeft(2));
}

bool DualEqKnob::GainSlider::hitTest(int x, int y) {
    const auto centre = getLocalBounds().toFloat().getCentre();
    const float radius = static_cast<float>(std::min(getWidth(), getHeight())) * 0.5f;
    return juce::Point<float>(static_cast<float>(x), static_cast<float>(y)).getDistanceFrom(centre) <= radius;
}

DualEqKnob::DualEqKnob(Apvts& state, const juce::String& freqId, const juce::String& dbId,
                       const juce::String& title)
    : state_(state), freqId_(freqId), dbId_(dbId), dbLock_(state, dbId), freqLock_(state, freqId) {
    setComponentID(dbId);
    title_.setText(title, juce::dontSendNotification);
    title_.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    title_.setJustificationType(juce::Justification::centred);
    title_.setColour(juce::Label::textColourId, theme::ink);
    addAndMakeVisible(title_);

    dbSlider_.setName(title + " Gain");
    dbSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dbSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    dbSlider_.setColour(juce::Slider::rotarySliderFillColourId, theme::violet);
    if (auto* dbParam = state.getParameter(dbId)) {
        dbSlider_.setDoubleClickReturnValue(true, dbParam->convertFrom0to1(dbParam->getDefaultValue()));
    }
    dbAttachment_ = std::make_unique<Apvts::SliderAttachment>(state, dbId, dbSlider_);
    dbSlider_.onValueChange = [this] { updateDbReadout(); };
    addAndMakeVisible(dbSlider_);

    dbReadout_.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    dbReadout_.setJustificationType(juce::Justification::centred);
    dbReadout_.setColour(juce::Label::textColourId, theme::ink);
    dbReadout_.setColour(juce::Label::backgroundColourId, theme::paper);
    dbReadout_.setEditable(false, true, false);
    dbReadout_.onTextChange = [this] {
        auto* dbParam = state_.getParameter(dbId_);
        if (dbParam == nullptr) return;
        const float value = dbParam->convertFrom0to1(dbParam->getValueForText(dbReadout_.getText()));
        dbSlider_.setValue(value, juce::sendNotificationSync);
    };
    addAndMakeVisible(dbReadout_);

    if (auto* freqParam = state.getParameter(freqId)) {
        freqHz_ = freqParam->convertFrom0to1(freqParam->getValue());
        freqAttachment_ = std::make_unique<juce::ParameterAttachment>(*freqParam, [this](float value) {
            freqHz_ = value;
            repaint();
        });
        freqAttachment_->sendInitialUpdate();
    }
    addAndMakeVisible(dbLock_);
    addAndMakeVisible(freqLock_);
    updateDbReadout();
}

void DualEqKnob::updateDbReadout() {
    const auto* dbParam = state_.getParameter(dbId_);
    dbReadout_.setText(dbParam->getText(dbParam->convertTo0to1(static_cast<float>(dbSlider_.getValue())), 0),
                       juce::dontSendNotification);
}

juce::Rectangle<float> DualEqKnob::dialBounds() const {
    const auto middle = getLocalBounds().withTrimmedTop(17);
    const float size = static_cast<float>(std::min(72, std::min(middle.getWidth() - 68, middle.getHeight())));
    return juce::Rectangle<float>(static_cast<float>(middle.getX()) + 2.0f,
                                  middle.toFloat().getCentreY() - size * 0.5f,
                                  size, size);
}

juce::Rectangle<int> DualEqKnob::dbPillBounds() const {
    const auto middle = getLocalBounds().withTrimmedTop(17);
    const int left = juce::roundToInt(dialBounds().getRight()) + 4;
    const int width = std::max(0, middle.getRight() - left - 2);
    return { left, middle.getY() + 2, width, 20 };
}

juce::Rectangle<int> DualEqKnob::freqPillBounds() const {
    const auto middle = getLocalBounds().withTrimmedTop(17);
    const int left = juce::roundToInt(dialBounds().getRight()) + 4;
    const int width = std::max(0, middle.getRight() - left - 2);
    return { left, middle.getY() + 25, width, 20 };
}

bool DualEqKnob::hitsOuterRing(juce::Point<float> pos) const {
    const auto dial = dialBounds();
    const float dist = pos.getDistanceFrom(dial.getCentre());
    const float outerR = dial.getWidth() * 0.5f + 3.0f;
    const float innerR = dial.getWidth() * 0.28f;
    return (dist >= innerR && dist <= outerR) || freqPillBounds().toFloat().contains(pos);
}

void DualEqKnob::paint(juce::Graphics& g) {
    const auto dial = dialBounds();
    const auto centre = dial.getCentre();
    const float outerR = dial.getWidth() * 0.5f - 2.0f;
    constexpr float startAngle = juce::MathConstants<float>::pi * 1.25f;
    constexpr float endAngle = juce::MathConstants<float>::pi * 2.75f;

    auto* freqParam = state_.getParameter(freqId_);
    const float norm = freqParam != nullptr ? freqParam->convertTo0to1(freqHz_) : 0.5f;
    const float angle = startAngle + norm * (endAngle - startAngle);
    const auto ringColour = (hoveringFreq_ || draggingFreq_) ? theme::apricot.brighter(0.2f) : theme::apricot;

    juce::Path base;
    base.addCentredArc(centre.x, centre.y, outerR, outerR, 0.0f, startAngle, endAngle, true);
    g.setColour(theme::edge);
    g.strokePath(base, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc(centre.x, centre.y, outerR, outerR, 0.0f, startAngle, angle, true);
    g.setColour(ringColour);
    g.strokePath(value, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float dotX = centre.x + outerR * std::sin(angle);
    const float dotY = centre.y - outerR * std::cos(angle);
    g.setColour(theme::ink);
    g.fillEllipse(dotX - 2.2f, dotY - 2.2f, 4.4f, 4.4f);

    const auto freqPill = freqPillBounds().toFloat();
    g.setColour(theme::paper.brighter(hoveringFreq_ || draggingFreq_ ? 0.1f : 0.0f));
    g.fillRoundedRectangle(freqPill, 4.0f);
    g.setColour(ringColour.withAlpha(hoveringFreq_ || draggingFreq_ ? 0.9f : 0.45f));
    g.drawRoundedRectangle(freqPill.reduced(0.5f), 4.0f, 1.0f);
    g.setColour(theme::ink);
    g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    if (freqParam != nullptr)
        g.drawFittedText(freqParam->getText(norm, 0), freqPill.toNearestInt(), juce::Justification::centred, 1);

    g.setColour(theme::secondary);
    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.drawText("dB", dbLock_.getBounds().withY(dbLock_.getBottom() - 2).withHeight(10), juce::Justification::centred);
    g.drawText("Hz", freqLock_.getBounds().withY(freqLock_.getBottom() - 2).withHeight(10), juce::Justification::centred);
}

void DualEqKnob::resized() {
    auto area = getLocalBounds();
    title_.setBounds(area.removeFromTop(17));
    const auto dial = dialBounds();
    const float innerSize = std::max(44.0f, dial.getWidth() * 0.68f);
    dbSlider_.setBounds(juce::Rectangle<float>(innerSize, innerSize).withCentre(dial.getCentre()).toNearestInt());
    dbReadout_.setBounds(dbPillBounds());
    const int locksLeft = juce::roundToInt(dial.getRight()) + 6;
    const int lockY = freqPillBounds().getBottom() + 4;
    dbLock_.setBounds(locksLeft, lockY, 22, 20);
    freqLock_.setBounds(locksLeft + 26, lockY, 22, 20);
}

void DualEqKnob::mouseMove(const juce::MouseEvent& event) {
    const bool over = hitsOuterRing(event.position);
    if (over != hoveringFreq_) {
        hoveringFreq_ = over;
        repaint();
    }
}

void DualEqKnob::mouseExit(const juce::MouseEvent&) {
    if (!hoveringFreq_) return;
    hoveringFreq_ = false;
    repaint();
}

void DualEqKnob::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isPopupMenu() || !hitsOuterRing(event.position)) return;
    auto* param = state_.getParameter(freqId_);
    if (param == nullptr || freqAttachment_ == nullptr) return;
    draggingFreq_ = true;
    dragStartNorm_ = param->convertTo0to1(freqHz_);
    dragStartY_ = event.position.y;
    freqAttachment_->beginGesture();
    repaint();
}

void DualEqKnob::mouseDrag(const juce::MouseEvent& event) {
    if (!draggingFreq_ || freqAttachment_ == nullptr) return;
    auto* param = state_.getParameter(freqId_);
    if (param == nullptr) return;
    const float norm = std::clamp(dragStartNorm_ + (dragStartY_ - event.position.y) / 140.0f, 0.0f, 1.0f);
    const float nextHz = param->convertFrom0to1(norm);
    freqHz_ = nextHz;
    freqAttachment_->setValueAsPartOfGesture(nextHz);
    repaint();
}

void DualEqKnob::mouseUp(const juce::MouseEvent&) {
    if (!draggingFreq_ || freqAttachment_ == nullptr) return;
    draggingFreq_ = false;
    freqAttachment_->endGesture();
    repaint();
}

void DualEqKnob::mouseDoubleClick(const juce::MouseEvent& event) {
    if (!hitsOuterRing(event.position) || freqAttachment_ == nullptr) return;
    auto* param = state_.getParameter(freqId_);
    if (param == nullptr) return;
    const float defaultHz = param->convertFrom0to1(param->getDefaultValue());
    freqHz_ = defaultHz;
    freqAttachment_->setValueAsCompleteGesture(defaultHz);
    repaint();
}

void DualEqKnob::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
    if (!hitsOuterRing(event.position) || freqAttachment_ == nullptr) return;
    auto* param = state_.getParameter(freqId_);
    if (param == nullptr) return;
    const float nextNorm = std::clamp(param->convertTo0to1(freqHz_) + wheel.deltaY * 0.08f, 0.0f, 1.0f);
    const float nextHz = param->convertFrom0to1(nextNorm);
    freqHz_ = nextHz;
    freqAttachment_->setValueAsCompleteGesture(nextHz);
    repaint();
}

} // namespace openclapper::ui
