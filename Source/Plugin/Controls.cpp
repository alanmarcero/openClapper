#include "Controls.h"
#include "Parameters.h"
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

LockBadge::LockBadge(Apvts& state, const juce::String& parameterId, const juce::String& lockedName)
    : state_(state), id_(parameterId) {
    setComponentID("lock_" + id_);
    setTitle("Lock " + lockedName);
    setDescription(state_.getParameter(id_)->getName(80));
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    setTooltip("Lock or unlock this setting for factory presets.");
    setTriggeredOnMouseDown(true);
    timerCallback();
    startTimerHz(10);
}

LockBadge::~LockBadge() { stopTimer(); }

void LockBadge::timerCallback() {
    setToggleState(static_cast<OpenClapperProcessor&>(state_.processor).isParamLocked(id_), juce::dontSendNotification);
}

void LockBadge::clicked() {
    static_cast<OpenClapperProcessor&>(state_.processor).setParamLocked(id_, getToggleState());
}

// JUCE buttons answer only Return, and only on the next message loop pass.
bool LockBadge::keyPressed(const juce::KeyPress& key) {
    if (!key.isKeyCode(juce::KeyPress::spaceKey) && !key.isKeyCode(juce::KeyPress::returnKey)) return false;
    setToggleState(!getToggleState(), juce::sendNotification);
    return true;
}

void LockBadge::paintButton(juce::Graphics& g, bool highlighted, bool) {
    const bool locked = getToggleState();
    const auto cx = static_cast<float>(getWidth()) * 0.5f;
    const auto cy = static_cast<float>(getHeight()) * 0.5f;
    const auto colour = locked ? theme::violet : theme::secondary;
    g.setColour(colour.withAlpha(locked || highlighted ? 1.0f : 0.6f));
    juce::Path shackle;
    shackle.startNewSubPath(cx - 3.5f, cy - 1.0f);
    shackle.lineTo(cx - 3.5f, cy - 4.0f);
    shackle.cubicTo(cx - 3.5f, cy - 8.5f, cx + 3.5f, cy - 8.5f, cx + 3.5f, cy - 4.0f);
    if (locked) shackle.lineTo(cx + 3.5f, cy - 1.0f);
    g.strokePath(shackle, juce::PathStrokeType(1.5f));
    g.drawRoundedRectangle(cx - 5.5f, cy - 1.5f, 11.0f, 8.5f, 1.8f, 1.5f);
    if (!hasKeyboardFocus(false)) return;
    g.setColour(theme::violet);
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 4.0f, 1.0f);
}

Control::Control(Apvts& state, const char* id, const char* caption)
    : lock_(std::make_unique<LockBadge>(state, id, caption)) {
    setComponentID(id);
    addAndMakeVisible(*lock_);
    auto* parameter = state.getParameter(id);
    jassert(parameter != nullptr);
    if (dynamic_cast<juce::AudioParameterBool*>(parameter) != nullptr) {
        toggle_ = std::make_unique<juce::TextButton>();
        toggle_->setName(parameter->getName(80));
        toggle_->setTitle(caption);
        // The host name tells a screen reader which layer a Mute or Solo button belongs to.
        toggle_->setDescription(parameter->getName(80));
        toggle_->setTooltip(parameter->getName(80));
        toggle_->setClickingTogglesState(true);
        const bool mute = juce::String(id).endsWith("_mute");
        toggle_->setColour(juce::TextButton::buttonOnColourId, mute ? theme::apricot : theme::violet);
        toggle_->setColour(juce::TextButton::textColourOnId, theme::plot);
        toggle_->onStateChange = [this, text = juce::String(caption)] {
            toggle_->setButtonText(text + (toggle_->getToggleState() ? " on" : " off"));
        };
        addAndMakeVisible(*toggle_);
        toggleAttachment_ = std::make_unique<Apvts::ButtonAttachment>(state, id, *toggle_);
        toggle_->onStateChange();
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
        choice_->setTitle(caption);
        choice_->setTooltip(parameter->getName(80));
        choice_->addItemList(choice->choices, 1);
        addAndMakeVisible(*choice_);
        choiceAttachment_ = std::make_unique<Apvts::ComboBoxAttachment>(state, id, *choice_);
        return;
    }

    slider_ = std::make_unique<FineSlider>();
    slider_->setName(parameter->getName(80));
    slider_->setTitle(caption);
    slider_->setTooltip(parameter->getName(80));
    slider_->setSliderStyle(juce::Slider::LinearHorizontal);
    slider_->setColour(juce::Slider::textBoxTextColourId, theme::ink);
    slider_->setColour(juce::Slider::textBoxBackgroundColourId, theme::paper);
    slider_->setColour(juce::Slider::textBoxOutlineColourId, theme::edge);
    slider_->setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 22);
    addAndMakeVisible(*slider_);
    sliderAttachment_ = std::make_unique<Apvts::SliderAttachment>(state, id, *slider_);
}

void Control::setInactiveReason(const juce::String& reason) {
    setAlpha(reason.isEmpty() ? 1.0f : theme::dimmedAlpha);
    label_.setTooltip(reason);
    const auto applyTooltip = [&](juce::Component* target) {
        if (target == nullptr) return;
        if (auto* client = dynamic_cast<juce::SettableTooltipClient*>(target))
            client->setTooltip(reason.isEmpty() ? target->getName() : reason);
    };
    applyTooltip(slider_.get());
    applyTooltip(choice_.get());
    applyTooltip(toggle_.get());
}

void Control::resized() {
    auto area = getLocalBounds().reduced(4, 1);
    if (toggleAttachment_) {
        const int toggleWidth = juce::jmin(144, juce::jmax(0, area.getWidth() - 26));
        const int pairWidth = toggleWidth + 4 + 22;
        auto pair = area.withSizeKeepingCentre(pairWidth, juce::jmin(36, area.getHeight()));
        toggle_->setBounds(pair.removeFromLeft(toggleWidth));
        pair.removeFromLeft(4);
        lock_->setBounds(pair.withSizeKeepingCentre(22, 22));
        return;
    }
    lock_->setBounds(area.removeFromRight(24).withSizeKeepingCentre(22, 22));
    area.removeFromRight(10);
    const int contentHeight = choiceAttachment_ ? 54 : 44;
    area = area.withSizeKeepingCentre(area.getWidth(), juce::jmin(contentHeight, area.getHeight()));
    label_.setBounds(area.removeFromTop(18));
    lock_->setBounds(lock_->getBounds().withY(area.getCentreY() - 11));
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
      lock_(state, "amp_enabled", "One-shot or Hold notes") {
    setComponentID("amp_enabled");
    oneShot_.setTooltip("Play each layer through its one-shot decay.");
    held_.setTooltip("Sustain the body and noise until a matching MIDI note-off.");
    oneShot_.setTitle("One-shot");
    held_.setTitle("Hold notes");
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
    : state_(state), freqId_(freqId), dbId_(dbId), dbLock_(state, dbId, title + " gain"),
      freqLock_(state, freqId, title + " frequency") {
    setComponentID(dbId);
    title_.setText(title, juce::dontSendNotification);
    title_.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    title_.setJustificationType(juce::Justification::centred);
    title_.setColour(juce::Label::textColourId, theme::ink);
    addAndMakeVisible(title_);

    dbSlider_.setName(title + " Gain");
    dbSlider_.setTitle(title + " gain");
    dbSlider_.setTooltip(title + " gain (dB)");
    dbSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dbSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    dbSlider_.setColour(juce::Slider::rotarySliderFillColourId, theme::violet);
    if (auto* dbParam = state.getParameter(dbId)) {
        dbSlider_.setDoubleClickReturnValue(true, dbParam->convertFrom0to1(dbParam->getDefaultValue()));
    }
    dbAttachment_ = std::make_unique<Apvts::SliderAttachment>(state, dbId, dbSlider_);
    dbSlider_.onValueChange = [this] { updateDbReadout(); };
    addAndMakeVisible(dbSlider_);

    dbReadout_.setTitle(title + " gain readout");
    dbReadout_.setTooltip(title + " gain (dB)");
    dbReadout_.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    dbReadout_.setBorderSize({ 0, 2, 0, 2 });
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

    freqReadout_.setTitle(title + " frequency readout");
    freqReadout_.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    freqReadout_.setBorderSize({ 0, 2, 0, 2 });
    freqReadout_.setJustificationType(juce::Justification::centred);
    freqReadout_.setColour(juce::Label::textColourId, theme::ink);
    freqReadout_.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    freqReadout_.setColour(juce::Label::backgroundWhenEditingColourId, theme::paper);
    freqReadout_.setColour(juce::Label::textWhenEditingColourId, theme::ink);
    freqReadout_.setColour(juce::Label::outlineWhenEditingColourId, theme::violet);
    freqReadout_.setMouseCursor(juce::MouseCursor::IBeamCursor);
    freqReadout_.setEditable(false, true, false);
    freqReadout_.onTextChange = [this] {
        auto* freqParam = state_.getParameter(freqId_);
        if (freqParam == nullptr || freqAttachment_ == nullptr) return;
        const float nextHz = freqParam->convertFrom0to1(freqParam->getValueForText(freqReadout_.getText()));
        freqHz_ = nextHz;
        freqAttachment_->setValueAsCompleteGesture(nextHz);
        updateFreqReadout();
        repaint();
    };
    addAndMakeVisible(freqReadout_);
    freqReadout_.addMouseListener(this, false);

    if (auto* freqParam = state.getParameter(freqId)) {
        freqHz_ = freqParam->convertFrom0to1(freqParam->getValue());
        freqAttachment_ = std::make_unique<juce::ParameterAttachment>(*freqParam, [this](float value) {
            freqHz_ = value;
            updateFreqReadout();
            repaint();
        });
        freqAttachment_->sendInitialUpdate();
    }
    addAndMakeVisible(dbLock_);
    addAndMakeVisible(freqLock_);
    updateDbReadout();
    updateFreqReadout();
}

void DualEqKnob::updateDbReadout() {
    dbReadout_.setText(dbSlider_.getTextFromValue(dbSlider_.getValue()), juce::dontSendNotification);
}

void DualEqKnob::updateFreqReadout() {
    auto* freqParam = state_.getParameter(freqId_);
    if (freqParam == nullptr) return;
    const float norm = freqParam->convertTo0to1(freqHz_);
    const auto note = freqParam->getText(norm, 0);
    freqReadout_.setText(note, juce::dontSendNotification);
    const char* range = "A0 to A2";
    if (freqId_ == "eq_mid_freq") range = "A3 to A5";
    if (freqId_ == "eq_high_freq") range = "A6 to A8";
    const auto tip = title_.getText() + " steps chromatic notes from " + range
        + ". Now " + note + " (" + juce::String(freqHz_, 1) + " Hz)";
    setTooltip(tip);
    freqReadout_.setTooltip(tip + ". Double-click to type a note or Hz. Right-click to pick a note.");
}

juce::Rectangle<float> DualEqKnob::dialBounds() const {
    const auto middle = getLocalBounds().withTrimmedTop(16);
    const float size = static_cast<float>(std::min(70, std::min(middle.getWidth() - 74, middle.getHeight())));
    return juce::Rectangle<float>(static_cast<float>(middle.getX()) + 2.0f,
                                  middle.toFloat().getCentreY() - size * 0.5f,
                                  size, size);
}

juce::Rectangle<int> DualEqKnob::dbPillBounds() const {
    const auto middle = getLocalBounds().withTrimmedTop(16);
    const int left = juce::roundToInt(dialBounds().getRight()) + 4;
    const int width = std::max(0, middle.getRight() - left - 1);
    return { left, middle.getY(), width, 19 };
}

juce::Rectangle<int> DualEqKnob::freqPillBounds() const {
    const auto middle = getLocalBounds().withTrimmedTop(16);
    const int left = juce::roundToInt(dialBounds().getRight()) + 4;
    const int width = std::max(0, middle.getRight() - left - 1);
    return { left, middle.getY() + 21, width, 19 };
}

bool DualEqKnob::hitsOuterRing(juce::Point<float> pos) const {
    const auto dial = dialBounds();
    const float dist = pos.getDistanceFrom(dial.getCentre());
    const float outerR = dial.getWidth() * 0.5f + 3.0f;
    const float innerR = dial.getWidth() * 0.28f;
    return dist >= innerR && dist <= outerR;
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

    const auto [minMidi, maxMidi] = params::eqBandMidiLimits(freqId_);
    const int semitoneSpan = std::max(1, maxMidi - minMidi);
    g.setColour(theme::secondary.withAlpha(0.55f));
    for (int step = 0; step <= semitoneSpan; ++step) {
        const float markNorm = static_cast<float>(step) / static_cast<float>(semitoneSpan);
        const float markAngle = startAngle + markNorm * (endAngle - startAngle);
        const bool octaveRoot = ((minMidi + step) % 12) == 9;
        const float innerTick = outerR - (octaveRoot ? 4.0f : 2.0f);
        const float outerTick = outerR + (octaveRoot ? 4.0f : 2.0f);
        g.drawLine(centre.x + innerTick * std::sin(markAngle), centre.y - innerTick * std::cos(markAngle),
                   centre.x + outerTick * std::sin(markAngle), centre.y - outerTick * std::cos(markAngle),
                   octaveRoot ? 1.2f : 0.75f);
    }

    const float dotX = centre.x + outerR * std::sin(angle);
    const float dotY = centre.y - outerR * std::cos(angle);
    g.setColour(theme::ink);
    g.fillEllipse(dotX - 2.2f, dotY - 2.2f, 4.4f, 4.4f);

    const auto freqPill = freqPillBounds().toFloat();
    g.setColour(theme::paper.brighter(hoveringFreq_ || draggingFreq_ ? 0.1f : 0.0f));
    g.fillRoundedRectangle(freqPill, 4.0f);
    g.setColour(ringColour.withAlpha(hoveringFreq_ || draggingFreq_ ? 0.9f : 0.45f));
    g.drawRoundedRectangle(freqPill.reduced(0.5f), 4.0f, 1.0f);

    g.setColour(theme::secondary);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("dB", dbLock_.getBounds().withSizeKeepingCentre(30, 15).withY(dbLock_.getBottom() + 1),
               juce::Justification::centred);
    g.drawText("Note", freqLock_.getBounds().withSizeKeepingCentre(36, 15).withY(freqLock_.getBottom() + 1),
               juce::Justification::centred);
}

void DualEqKnob::resized() {
    auto area = getLocalBounds();
    title_.setBounds(area.removeFromTop(16));
    const auto dial = dialBounds();
    const float innerSize = std::max(44.0f, dial.getWidth() * 0.68f);
    dbSlider_.setBounds(juce::Rectangle<float>(innerSize, innerSize).withCentre(dial.getCentre()).toNearestInt());
    dbReadout_.setBounds(dbPillBounds());
    freqReadout_.setBounds(freqPillBounds());
    const int locksLeft = juce::roundToInt(dial.getRight()) + 8;
    const int lockY = freqPillBounds().getBottom() + 4;
    dbLock_.setBounds(locksLeft, lockY, 22, 18);
    freqLock_.setBounds(locksLeft + 30, lockY, 22, 18);
}

void DualEqKnob::mouseMove(const juce::MouseEvent& event) {
    const bool over = hitsOuterRing(event.position);
    setMouseCursor(over ? juce::MouseCursor::UpDownResizeCursor : juce::MouseCursor::NormalCursor);
    if (over != hoveringFreq_) {
        hoveringFreq_ = over;
        repaint();
    }
}

void DualEqKnob::mouseExit(const juce::MouseEvent&) {
    setMouseCursor(juce::MouseCursor::NormalCursor);
    if (!hoveringFreq_) return;
    hoveringFreq_ = false;
    repaint();
}

void DualEqKnob::mouseDown(const juce::MouseEvent& event) {
    if (event.mods.isPopupMenu()) {
        if (event.eventComponent != &freqReadout_ || freqAttachment_ == nullptr) return;
        auto* param = state_.getParameter(freqId_);
        if (param == nullptr) return;
        const auto [minMidi, maxMidi] = params::eqBandMidiLimits(freqId_);
        const int current = suite::eq::nearestMidi(freqHz_, minMidi, maxMidi);
        auto menu = suite::eq::noteMenu(minMidi, maxMidi, current, 1);
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&freqReadout_),
            [this, minMidi, maxMidi](int itemId) {
                const auto midi = suite::eq::midiFromNoteItem(itemId, 1, minMidi, maxMidi);
                if (!midi || freqAttachment_ == nullptr) return;
                const float hz = suite::eq::noteToHz(*midi);
                freqHz_ = hz;
                freqAttachment_->setValueAsCompleteGesture(hz);
                updateFreqReadout();
                repaint();
            });
        return;
    }
    if (!hitsOuterRing(event.position)) return;
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
    if (param == nullptr || wheel.deltaY == 0.0f) return;
    const auto [minMidi, maxMidi] = params::eqBandMidiLimits(freqId_);
    const float semitoneStep = 1.0f / static_cast<float>(std::max(1, maxMidi - minMidi));
    const float direction = wheel.deltaY > 0.0f ? 1.0f : -1.0f;
    const float nextNorm = std::clamp(param->convertTo0to1(freqHz_) + direction * semitoneStep, 0.0f, 1.0f);
    const float nextHz = param->convertFrom0to1(nextNorm);
    freqHz_ = nextHz;
    freqAttachment_->setValueAsCompleteGesture(nextHz);
    repaint();
}

} // namespace openclapper::ui
