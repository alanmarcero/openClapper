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

void EdmLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                           const juce::Colour& background, bool hover, bool down) {
    auto fill = background;
    if (hover || down) fill = fill.interpolatedWith(theme::violet, down ? 0.23f : 0.10f);
    const auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    g.setColour(fill);
    g.fillRoundedRectangle(bounds, 6);
    g.setColour(button.hasKeyboardFocus(false) ? theme::violet : theme::edge.withAlpha(0.75f));
    g.drawRoundedRectangle(bounds, 6, 1);
}

juce::Font EdmLookAndFeel::getTextButtonFont(juce::TextButton&, int) {
    return juce::FontOptions(15.0f, juce::Font::bold);
}

juce::Font EdmLookAndFeel::getComboBoxFont(juce::ComboBox&) { return juce::FontOptions(16.0f); }

} // namespace openclapper::ui
