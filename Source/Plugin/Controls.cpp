#include "Controls.h"
#include "Theme.h"

namespace openclapper::ui {

Control::Control(Apvts& state, const char* id, const char* caption) {
    setComponentID(id);
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

    slider_ = std::make_unique<juce::Slider>();
    slider_->setName(parameter->getName(80));
    slider_->setSliderStyle(juce::Slider::LinearHorizontal);
    slider_->setColour(juce::Slider::textBoxTextColourId, theme::ink);
    slider_->setColour(juce::Slider::textBoxBackgroundColourId, theme::paper);
    slider_->setColour(juce::Slider::textBoxOutlineColourId, theme::edge);
    slider_->setTextBoxStyle(juce::Slider::TextBoxRight, false, 74, 22);
    addAndMakeVisible(*slider_);
    sliderAttachment_ = std::make_unique<Apvts::SliderAttachment>(state, id, *slider_);
    const auto unit = parameter->getLabel().isEmpty() && juce::String(id).startsWith("amp_")
            && juce::String(id) != "amp_sustain" ? juce::String("ms") : parameter->getLabel();
    const auto step = parameter->getNormalisableRange().interval;
    slider_->textFromValueFunction = [unit, step](double v) {
        const int digits = step >= 1.0f ? 0 : step >= 0.1f ? 1 : 2;
        return juce::String(std::abs(v) < 0.5 * step ? 0.0 : v, digits) + (unit.isEmpty() ? juce::String() : " " + unit);
    };
    slider_->valueFromTextFunction = [](const juce::String& s) { return s.getDoubleValue(); };
    slider_->updateText();
}

void Control::resized() {
    auto area = getLocalBounds().reduced(5, 1);
    if (toggleAttachment_) {
        toggle_->setBounds(area.withSizeKeepingCentre(juce::jmin(148, area.getWidth()), juce::jmin(36, area.getHeight())));
        return;
    }
    const int contentHeight = choiceAttachment_ ? 54 : 44;
    area = area.withSizeKeepingCentre(area.getWidth(), juce::jmin(contentHeight, area.getHeight()));
    label_.setBounds(area.removeFromTop(18));
    if (choiceAttachment_) choice_->setBounds(area);
    else slider_->setBounds(area);
}

NoteGate::NoteGate(Apvts& state)
    : attachment_(*state.getParameter("amp_enabled"), [this](float value) {
        oneShot_.setToggleState(value < 0.5f, juce::dontSendNotification);
        held_.setToggleState(value >= 0.5f, juce::dontSendNotification);
    }) {
    setComponentID("amp_enabled");
    oneShot_.setTooltip("Play each layer through its one-shot decay.");
    held_.setTooltip("Sustain the body and noise until a matching MIDI note-off.");
    oneShot_.onClick = [this] { attachment_.setValueAsCompleteGesture(0.0f); };
    held_.onClick = [this] { attachment_.setValueAsCompleteGesture(1.0f); };
    addAndMakeVisible(oneShot_);
    addAndMakeVisible(held_);
    attachment_.sendInitialUpdate();
}

void NoteGate::resized() {
    auto bounds = getLocalBounds().reduced(5, 0);
    bounds = bounds.withSizeKeepingCentre(bounds.getWidth(), 36);
    oneShot_.setBounds(bounds.removeFromLeft(bounds.getWidth() / 2).withTrimmedRight(2));
    held_.setBounds(bounds.withTrimmedLeft(2));
}

} // namespace openclapper::ui
