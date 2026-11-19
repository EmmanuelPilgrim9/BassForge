#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include "BassEngine.h"

namespace ParamID {
    inline constexpr const char* frequency   = "frequency";
    inline constexpr const char* intensity   = "intensity";
    inline constexpr const char* character   = "character";
    inline constexpr const char* fundamental = "fundamental";
    inline constexpr const char* output      = "output";
    inline constexpr const char* solo        = "solo";
}

class BassForgeProcessor : public juce::AudioProcessor {
public:
    BassForgeProcessor();
    ~BassForgeProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    static constexpr int kMaxChannels = 2;
    std::array<bassforge::ChannelEngine, kMaxChannels> engines;
    bassforge::Smoothed frequencySm, intensitySm, characterSm, fundamentalSm, gainSm;

    std::atomic<float>* frequencyParam   = nullptr;
    std::atomic<float>* intensityParam   = nullptr;
    std::atomic<float>* characterParam   = nullptr;
    std::atomic<float>* fundamentalParam = nullptr;
    std::atomic<float>* outputParam      = nullptr;
    std::atomic<float>* soloParam        = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BassForgeProcessor)
};
