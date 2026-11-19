#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class BassForgeLookAndFeel : public juce::LookAndFeel_V4 {
public:
    BassForgeLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h,
                          float pos, float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
};

struct LabeledKnob : public juce::Component {
    LabeledKnob(juce::AudioProcessorValueTreeState& state, const juce::String& id, const juce::String& title);
    void resized() override;

    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class BassForgeEditor : public juce::AudioProcessorEditor {
public:
    explicit BassForgeEditor(BassForgeProcessor&);
    ~BassForgeEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    BassForgeLookAndFeel lnf;
    LabeledKnob frequency, intensity, character, fundamental, output;
    juce::ToggleButton soloButton { "Solo Harmonics" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> soloAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BassForgeEditor)
};
