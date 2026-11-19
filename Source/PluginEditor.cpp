#include "PluginEditor.h"

namespace Colours {
    const juce::Colour background { 0xff15161a };
    const juce::Colour panel      { 0xff1f2127 };
    const juce::Colour track      { 0xff2e313a };
    const juce::Colour accent     { 0xffff7a1a };
    const juce::Colour text       { 0xffe8e8ea };
    const juce::Colour dim        { 0xff8a8d96 };
}

// ── LookAndFeel ────────────────────────────────────────────────────────────────

BassForgeLookAndFeel::BassForgeLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, Colours::text);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::textColourId, Colours::dim);
    setColour(juce::ToggleButton::textColourId, Colours::text);
}

void BassForgeLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                            float pos, float startAngle, float endAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<int>(x, y, w, h).toFloat().reduced(6.0f);
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float stroke = 5.0f;
    const float arcR = radius - stroke * 0.5f;

    juce::Path bg;
    bg.addCentredArc(centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour(Colours::track);
    g.strokePath(bg, juce::PathStrokeType(stroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc(centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);
    g.setColour(Colours::accent);
    g.strokePath(value, juce::PathStrokeType(stroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float knobR = arcR - stroke - 4.0f;
    g.setColour(Colours::panel.brighter(0.08f));
    g.fillEllipse(centre.x - knobR, centre.y - knobR, knobR * 2.0f, knobR * 2.0f);

    const juce::Point<float> tip(centre.x + (knobR - 6.0f) * std::sin(angle),
                                 centre.y - (knobR - 6.0f) * std::cos(angle));
    g.setColour(Colours::text);
    g.drawLine({ centre.getPointOnCircumference(knobR * 0.35f, angle), tip }, 3.0f);
}

void BassForgeLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced(1.0f);
    const bool on = b.getToggleState();
    g.setColour(on ? Colours::accent : (highlighted ? Colours::track.brighter(0.1f) : Colours::track));
    g.fillRoundedRectangle(r, 6.0f);
    g.setColour(on ? Colours::background : Colours::text);
    g.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
    g.drawFittedText(b.getButtonText(), r.toNearestInt(), juce::Justification::centred, 1);
}

// ── LabeledKnob ────────────────────────────────────────────────────────────────

LabeledKnob::LabeledKnob(juce::AudioProcessorValueTreeState& state, const juce::String& id, const juce::String& title)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 20);
    slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                               juce::MathConstants<float>::pi * 2.75f, true);
    addAndMakeVisible(slider);

    label.setText(title.toUpperCase(), juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    addAndMakeVisible(label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, id, slider);
}

void LabeledKnob::resized()
{
    auto r = getLocalBounds();
    label.setBounds(r.removeFromTop(20));
    slider.setBounds(r);
}

// ── Editor ─────────────────────────────────────────────────────────────────────

BassForgeEditor::BassForgeEditor(BassForgeProcessor& p)
    : AudioProcessorEditor(&p),
      frequency  (p.apvts, ParamID::frequency,   "Frequency"),
      intensity  (p.apvts, ParamID::intensity,   "Intensity"),
      character  (p.apvts, ParamID::character,   "Warm / Bite"),
      fundamental(p.apvts, ParamID::fundamental, "Fundamental"),
      output     (p.apvts, ParamID::output,      "Output")
{
    setLookAndFeel(&lnf);
    for (auto* k : { &frequency, &intensity, &character, &fundamental, &output })
        addAndMakeVisible(k);

    addAndMakeVisible(soloButton);
    soloAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        p.apvts, ParamID::solo, soloButton);

    setSize(640, 300);
}

BassForgeEditor::~BassForgeEditor()
{
    setLookAndFeel(nullptr);
}

void BassForgeEditor::paint(juce::Graphics& g)
{
    g.fillAll(Colours::background);

    auto header = getLocalBounds().removeFromTop(56).reduced(20, 0);
    g.setColour(Colours::text);
    g.setFont(juce::Font(juce::FontOptions(24.0f, juce::Font::bold)));
    g.drawText("BASSFORGE", header, juce::Justification::centredLeft);
    g.setColour(Colours::dim);
    g.setFont(juce::Font(juce::FontOptions(12.0f)));
    g.drawText("psychoacoustic bass enhancer", header.withTrimmedLeft(150), juce::Justification::centredLeft);

    g.setColour(Colours::panel);
    g.fillRoundedRectangle(getLocalBounds().withTrimmedTop(56).reduced(12).toFloat(), 10.0f);
}

void BassForgeEditor::resized()
{
    auto header = getLocalBounds().removeFromTop(56).reduced(20, 14);
    soloButton.setBounds(header.removeFromRight(140));

    auto area = getLocalBounds().withTrimmedTop(56).reduced(24);
    const int w = area.getWidth() / 5;
    for (auto* k : { &frequency, &intensity, &character, &fundamental, &output })
        k->setBounds(area.removeFromLeft(w).reduced(4));
}
