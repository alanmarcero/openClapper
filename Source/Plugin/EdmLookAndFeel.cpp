#include "EdmLookAndFeel.h"
#include "Theme.h"

namespace openclapper::ui {

EdmLookAndFeel::EdmLookAndFeel() {
    setColour(juce::TextButton::buttonColourId, theme::surface);
    setColour(juce::TextButton::buttonOnColourId, theme::selected);
    setColour(juce::TextButton::textColourOffId, theme::ink);
    setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    setColour(juce::ComboBox::backgroundColourId, theme::surface);
    setColour(juce::ComboBox::outlineColourId, theme::edge);
    setColour(juce::ComboBox::textColourId, theme::ink);
    setColour(juce::ComboBox::arrowColourId, theme::violet);
    setColour(juce::TooltipWindow::backgroundColourId, theme::plot);
    setColour(juce::TooltipWindow::textColourId, theme::ink);
    setColour(juce::TooltipWindow::outlineColourId, theme::edge);
    setColour(juce::TextEditor::backgroundColourId, theme::plot);
    setColour(juce::TextEditor::textColourId, theme::ink);
    setColour(juce::TextEditor::highlightColourId, theme::selected);
    setColour(juce::PopupMenu::backgroundColourId, theme::surface);
    setColour(juce::PopupMenu::textColourId, theme::ink);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, theme::selected);
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
}

void EdmLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float position,
                                      float, float, juce::Slider::SliderStyle, juce::Slider& slider) {
    const float centre = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    const float left = static_cast<float>(x);
    const auto active = slider.isEnabled() ? theme::violet : theme::edge;
    g.setColour(theme::edge);
    g.fillRoundedRectangle(left, centre - 2, static_cast<float>(width), 4, 2);
    g.setColour(active);
    g.fillRoundedRectangle(left, centre - 2, juce::jmax(0.0f, position - left), 4, 2);
    g.setColour(slider.isEnabled() ? theme::ink : theme::secondary);
    g.fillRoundedRectangle(position - 3, centre - 7, 6, 14, 2);
}

void EdmLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float position,
                                      float startAngle, float endAngle, juce::Slider& slider) {
    const auto area = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(3.0f);
    const float radius = std::min(area.getWidth(), area.getHeight()) * 0.5f;
    const auto centre = area.getCentre();
    const float arcRadius = std::max(4.0f, radius - 3.5f);
    const float angle = startAngle + position * (endAngle - startAngle);
    const bool bipolar = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;
    const float origin = bipolar ? (startAngle + endAngle) * 0.5f : startAngle;
    const auto stroke = juce::PathStrokeType(2.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
    const bool enabled = slider.isEnabled();
    if (!enabled) g.beginTransparencyLayer(theme::dimmedAlpha);

    juce::Path base;
    base.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour(theme::edge);
    g.strokePath(base, stroke);

    juce::Path value;
    value.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f, origin, angle, true);
    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    g.strokePath(value, stroke);

    const float capRadius = std::max(3.0f, arcRadius - 5.0f);
    const auto cap = juce::Rectangle<float>(capRadius * 2.0f, capRadius * 2.0f).withCentre(centre);
    g.setColour(theme::surface.brighter(0.08f));
    g.fillEllipse(cap);
    g.setColour(theme::edge);
    g.drawEllipse(cap, 1.0f);

    const float pointerLen = capRadius * 0.68f;
    g.setColour(theme::ink);
    g.drawLine(centre.x, centre.y, centre.x + pointerLen * std::sin(angle), centre.y - pointerLen * std::cos(angle), 1.8f);
    if (!enabled) g.endTransparencyLayer();
}

void EdmLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                           const juce::Colour& background, bool hover, bool down) {
    const bool activeSelected = button.getToggleState() && background == theme::selected;
    auto fill = activeSelected ? theme::selected.brighter(0.22f) : background;
    if (hover || down) fill = fill.interpolatedWith(theme::violet, down ? 0.23f : 0.10f);
    const float alpha = button.isEnabled() ? 1.0f : theme::dimmedAlpha;
    const auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    g.setColour(fill.withMultipliedAlpha(alpha));
    g.fillRoundedRectangle(bounds, 6);
    const auto borderColour = button.hasKeyboardFocus(false) ? theme::violet
                            : activeSelected               ? theme::violet.withAlpha(0.85f * alpha)
                                                           : theme::edge.withAlpha(0.75f * alpha);
    g.setColour(borderColour);
    g.drawRoundedRectangle(bounds, 6, activeSelected ? 1.5f : 1.0f);
    if (activeSelected && button.getComponentID().startsWith("tab_")) {
        g.setColour(theme::violet.withMultipliedAlpha(alpha));
        g.fillRoundedRectangle(bounds.getX() + 6.0f, bounds.getBottom() - 3.0f, bounds.getWidth() - 12.0f, 2.5f, 1.25f);
    }
}

void EdmLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool down, int buttonX, int buttonY,
                                  int buttonW, int buttonH, juce::ComboBox& box) {
    const bool enabled = box.isEnabled();
    if (!enabled) g.beginTransparencyLayer(theme::dimmedAlpha);
    LookAndFeel_V4::drawComboBox(g, width, height, down, buttonX, buttonY, buttonW, buttonH, box);
    if (!enabled) g.endTransparencyLayer();
}

juce::Font EdmLookAndFeel::getTextButtonFont(juce::TextButton&, int) {
    return juce::FontOptions(15.0f, juce::Font::bold);
}

juce::Font EdmLookAndFeel::getComboBoxFont(juce::ComboBox&) { return juce::FontOptions(16.0f); }

} // namespace openclapper::ui
