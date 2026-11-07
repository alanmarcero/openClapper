#include "EdmLookAndFeel.h"
#include "Theme.h"

namespace openclapper::ui {

namespace {

constexpr float kArcThickness = 3.5f;
constexpr float kCornerRadius = 4.5f;
constexpr float kControlFontSize = 12.0f;

} // namespace

EdmLookAndFeel::EdmLookAndFeel() {
    setColour(juce::ResizableWindow::backgroundColourId, theme::kBackground);
    setColour(juce::Label::textColourId, theme::kText);
    setColour(juce::Slider::textBoxTextColourId, theme::kText);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff1e2b3c));
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff090f17));
    setColour(juce::Slider::textBoxHighlightColourId, theme::kTrack);
    setColour(juce::ComboBox::backgroundColourId, theme::kKnobBody);
    setColour(juce::ComboBox::textColourId, theme::kText);
    setColour(juce::ComboBox::arrowColourId, theme::kAccentCyan);
    setColour(juce::PopupMenu::backgroundColourId, theme::kPanel);
    setColour(juce::PopupMenu::textColourId, theme::kText);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, theme::kTrack);
    setColour(juce::PopupMenu::highlightedTextColourId, theme::kAccentCyan);
    setColour(juce::TextButton::buttonColourId, theme::kKnobBody);
    setColour(juce::TextButton::textColourOffId, theme::kTextDim);
    setColour(juce::TextButton::textColourOnId, theme::kBackground);
}

void EdmLookAndFeel::drawRotarySlider(
    juce::Graphics& g,
    int x,
    int y,
    int width,
    int height,
    float sliderPos,
    float startAngle,
    float endAngle,
    juce::Slider& slider) {
    const auto accent = slider.findColour(juce::Slider::rotarySliderFillColourId);
    const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(3.5f);
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    const auto centre = bounds.getCentre();
    const float arcRadius = radius - kArcThickness;
    const float valueAngle = startAngle + sliderPos * (endAngle - startAngle);
    const bool isBipolar = juce::approximatelyEqual(slider.getMinimum(), -slider.getMaximum());
    const float originAngle = isBipolar ? (startAngle + endAngle) / 2.0f : startAngle;
    const auto stroke = juce::PathStrokeType(kArcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);

    juce::Path track;
    track.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour(theme::kTrack);
    g.strokePath(track, stroke);

    juce::Path value;
    value.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, originAngle, valueAngle, true);
    g.setColour(accent);
    g.strokePath(value, stroke);

    const float bodyRadius = arcRadius - kArcThickness - 1.5f;
    const auto body = juce::Rectangle<float>(bodyRadius * 2.0f, bodyRadius * 2.0f).withCentre(centre);
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillEllipse(body.translated(0.0f, 2.0f));
    g.setGradientFill(juce::ColourGradient(
        theme::kKnobBody.brighter(0.6f), centre.x, body.getY(),
        theme::kKnobBody.darker(0.35f), centre.x, body.getBottom(), false));
    g.fillEllipse(body);
    g.setColour(theme::kPanelEdge);
    g.drawEllipse(body, 1.0f);

    const auto tip = centre.getPointOnCircumference(bodyRadius - 2.5f, valueAngle);
    const auto tail = centre.getPointOnCircumference(bodyRadius * 0.35f, valueAngle);
    g.setColour(theme::kText);
    g.drawLine({ tail, tip }, 2.0f);
}

void EdmLookAndFeel::drawComboBox(
    juce::Graphics& g,
    int width,
    int height,
    bool,
    int,
    int,
    int,
    int,
    juce::ComboBox& box) {
    const auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f);
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(bounds, kCornerRadius);
    g.setColour(theme::kPanelEdge);
    g.drawRoundedRectangle(bounds, kCornerRadius, 1.0f);

    const auto arrowZone = juce::Rectangle<float>(static_cast<float>(width) - 20.0f, 0.0f, 16.0f, static_cast<float>(height));
    juce::Path arrow;
    arrow.startNewSubPath(arrowZone.getX() + 3.0f, arrowZone.getCentreY() - 2.0f);
    arrow.lineTo(arrowZone.getCentreX(), arrowZone.getCentreY() + 3.0f);
    arrow.lineTo(arrowZone.getRight() - 3.0f, arrowZone.getCentreY() - 2.0f);
    g.setColour(box.findColour(juce::ComboBox::arrowColourId));
    g.strokePath(arrow, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void EdmLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label) {
    label.setBounds(1, 1, box.getWidth() - 24, box.getHeight() - 2);
    label.setFont(getComboBoxFont(box));
}

void EdmLookAndFeel::drawButtonBackground(
    juce::Graphics& g,
    juce::Button& button,
    const juce::Colour& backgroundColour,
    bool isHighlighted,
    bool isDown) {
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    const auto baseColour = button.getToggleState()
        ? button.findColour(juce::TextButton::buttonOnColourId)
        : backgroundColour;
    const auto fill = isDown ? baseColour.darker(0.2f) : (isHighlighted ? baseColour.brighter(0.15f) : baseColour);

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, kCornerRadius);
    g.setColour(button.getToggleState() ? theme::kAccentCyan : theme::kPanelEdge);
    g.drawRoundedRectangle(bounds, kCornerRadius, 1.0f);
}

void EdmLookAndFeel::drawButtonText(
    juce::Graphics& g,
    juce::TextButton& button,
    bool,
    bool) {
    const auto colour = button.getToggleState()
        ? button.findColour(juce::TextButton::textColourOnId)
        : button.findColour(juce::TextButton::textColourOffId);
    g.setColour(colour);
    g.setFont(juce::FontOptions(kControlFontSize, juce::Font::bold));
    g.drawText(button.getButtonText(), button.getLocalBounds().reduced(4, 2),
               juce::Justification::centred, true);
}

juce::Font EdmLookAndFeel::getComboBoxFont(juce::ComboBox&) {
    return juce::FontOptions(kControlFontSize);
}

juce::Font EdmLookAndFeel::getPopupMenuFont() {
    return juce::FontOptions(kControlFontSize);
}

juce::Label* EdmLookAndFeel::createSliderTextBox(juce::Slider& slider) {
    auto* l = LookAndFeel_V4::createSliderTextBox(slider);
    l->setFont(juce::FontOptions(11.0f, juce::Font::bold));
    l->setJustificationType(juce::Justification::centred);
    l->setColour(juce::Label::backgroundColourId, juce::Colour(0xff090f17));
    l->setColour(juce::Label::outlineColourId, juce::Colour(0xff1e2b3c));
    return l;
}

} // namespace openclapper::ui
