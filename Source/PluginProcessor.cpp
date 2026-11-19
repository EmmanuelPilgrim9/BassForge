#include "PluginProcessor.h"
#include "PluginEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;

BassForgeProcessor::BassForgeProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "BassForge", createLayout())
{
    frequencyParam   = apvts.getRawParameterValue(ParamID::frequency);
    intensityParam   = apvts.getRawParameterValue(ParamID::intensity);
    characterParam   = apvts.getRawParameterValue(ParamID::character);
    fundamentalParam = apvts.getRawParameterValue(ParamID::fundamental);
    outputParam      = apvts.getRawParameterValue(ParamID::output);
    soloParam        = apvts.getRawParameterValue(ParamID::solo);
}

APVTS::ParameterLayout BassForgeProcessor::createLayout()
{
    using namespace juce;
    APVTS::ParameterLayout layout;

    auto hz = AudioParameterFloatAttributes().withLabel("Hz")
        .withStringFromValueFunction([](float v, int) { return String(v, 1) + " Hz"; });
    auto pct = AudioParameterFloatAttributes().withLabel("%")
        .withStringFromValueFunction([](float v, int) { return String(roundToInt(v)) + " %"; });
    auto db = AudioParameterFloatAttributes().withLabel("dB")
        .withStringFromValueFunction([](float v, int) { return String(v, 1) + " dB"; });

    NormalisableRange<float> freqRange(30.0f, 200.0f);
    freqRange.setSkewForCentre(70.0f);

    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ ParamID::frequency, 1 }, "Frequency",
        freqRange, 70.0f, hz));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ ParamID::intensity, 1 }, "Intensity",
        NormalisableRange<float>(0.0f, 100.0f), 50.0f, pct));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ ParamID::character, 1 }, "Character",
        NormalisableRange<float>(0.0f, 100.0f), 30.0f, pct));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ ParamID::fundamental, 1 }, "Fundamental",
        NormalisableRange<float>(0.0f, 100.0f), 100.0f, pct));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ ParamID::output, 1 }, "Output",
        NormalisableRange<float>(-24.0f, 12.0f, 0.1f), 0.0f, db));
    layout.add(std::make_unique<AudioParameterBool>(ParameterID{ ParamID::solo, 1 }, "Solo Harmonics", false));

    return layout;
}

bool BassForgeProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return out == layouts.getMainInputChannelSet();
}

void BassForgeProcessor::prepareToPlay(double sampleRate, int)
{
    for (auto& e : engines) e.prepare(sampleRate);

    frequencySm.prepare(sampleRate, 0.05, frequencyParam->load());
    intensitySm.prepare(sampleRate, 0.02, intensityParam->load() * 0.01);
    characterSm.prepare(sampleRate, 0.02, characterParam->load() * 0.01);
    fundamentalSm.prepare(sampleRate, 0.02, fundamentalParam->load() * 0.01);
    gainSm.prepare(sampleRate, 0.02, bassforge::dbToGain(outputParam->load()));

    for (auto& e : engines) e.updateFilters(frequencySm.current);
}

void BassForgeProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numIn  = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    for (int ch = numIn; ch < numOut; ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    frequencySm.setTarget(frequencyParam->load());
    intensitySm.setTarget(intensityParam->load() * 0.01);
    characterSm.setTarget(characterParam->load() * 0.01);
    fundamentalSm.setTarget(fundamentalParam->load() * 0.01);
    gainSm.setTarget(bassforge::dbToGain(outputParam->load()));
    const bool solo = soloParam->load() > 0.5f;

    const int numChannels = juce::jmin(numOut, kMaxChannels);
    const int numSamples  = buffer.getNumSamples();
    constexpr int kFilterUpdateInterval = 32; // recalcular filtros cada 32 muestras es suficiente

    for (int i = 0; i < numSamples; ++i)
    {
        const double freq = frequencySm.next();
        if (i % kFilterUpdateInterval == 0)
            for (int ch = 0; ch < numChannels; ++ch) engines[(size_t)ch].updateFilters(freq);

        const double inten = intensitySm.next();
        const double chr   = characterSm.next();
        const double fund  = fundamentalSm.next();
        const double gain  = gainSm.next();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            data[i] = (float)engines[(size_t)ch].process(data[i], chr, inten, fund, gain, solo);
        }
    }
}

void BassForgeProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void BassForgeProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* BassForgeProcessor::createEditor()
{
    return new BassForgeEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BassForgeProcessor();
}
