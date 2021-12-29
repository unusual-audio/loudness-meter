/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LoudnessMeterAudioProcessor::LoudnessMeterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
                  .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
                  .withOutput ("Output",    juce::AudioChannelSet::stereo(), true))
#endif
{
    addParameter (targetIntegratedLoudness = new juce::AudioParameterFloat ("targetIntegratedLoudness", "Integrated", juce::NormalisableRange<float>(-50.0f, 0.0f, 1.0f), -23.0f));
    addParameter (targetMaximumShortTermLoudness = new juce::AudioParameterFloat ("targetMaximumShortTermLoudness", "Max. short term", juce::NormalisableRange<float>(-50.0f, 0.0f, 1.0f), -18.0f));
    addParameter (targetMaximumTruePeakLevel = new juce::AudioParameterFloat ("targetMaximumTruePeakLevel", "Max. true peak", juce::NormalisableRange<float>(-50.0f, 6.0f, 1.0f),  -1.0f));
}

LoudnessMeterAudioProcessor::~LoudnessMeterAudioProcessor()
{
}

//==============================================================================
const juce::String LoudnessMeterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LoudnessMeterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LoudnessMeterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LoudnessMeterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LoudnessMeterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LoudnessMeterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LoudnessMeterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LoudnessMeterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LoudnessMeterAudioProcessor::getProgramName (int index)
{
    return {};
}

void LoudnessMeterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LoudnessMeterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    interpolatedBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock * oversampling);
}

void LoudnessMeterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LoudnessMeterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void LoudnessMeterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    loudnessMeter.processBlock(buffer);
    measurementPaused = loudnessMeter.getMomentaryLoudness() < -70;
    if (!measurementPaused)
    {
        integratedLoudness = loudnessMeter.getIntegratedLoudness();
    }
    shortTermLoudness = loudnessMeter.getShortTermLoudness();
    momentaryLoudness = loudnessMeter.getMomentaryLoudness();
    maximumShortTermLoudness = loudnessMeter.getMaximumShortTermLoudness();
    maximumMomentaryLoudness = loudnessMeter.getMaximumMomentaryLoudness();
    loudnessRange = loudnessMeter.getLoudnessRange();
    
    peakLevel = juce::Decibels::gainToDecibels(buffer.getMagnitude(0, buffer.getNumSamples()), -INFINITY);
    maximumPeakLevel = peakLevel > maximumPeakLevel ? peakLevel : maximumPeakLevel;
    
    truePeakLevel = juce::Decibels::gainToDecibels(truePeakProcessor.process(buffer).getMax(), -INFINITY);
    maximumTruePeakLevel = truePeakLevel > maximumTruePeakLevel ? truePeakLevel : maximumTruePeakLevel;

}

void LoudnessMeterAudioProcessor::resetIntegratedLoudness()
{
    loudnessMeter.reset();
    integratedLoudness = -INFINITY;
    maximumShortTermLoudness = -INFINITY;
    maximumMomentaryLoudness = -INFINITY;
    peakLevel = -INFINITY;
    maximumPeakLevel = -INFINITY;
    maximumTruePeakLevel = -INFINITY;
    truePeakLevel = -INFINITY;
}

//==============================================================================
bool LoudnessMeterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LoudnessMeterAudioProcessor::createEditor()
{
    return new LoudnessMeterAudioProcessorEditor (*this);
}

//==============================================================================
void LoudnessMeterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    std::unique_ptr<juce::XmlElement> xml (new juce::XmlElement ("LoudnessMeterParams"));
    xml->setAttribute ("targetIntegratedLoudness", (double) *targetIntegratedLoudness);
    xml->setAttribute ("targetMaximumShortTermLoudness", (double) *targetMaximumShortTermLoudness);
    xml->setAttribute ("targetMaximumTruePeakLevel", (double) *targetMaximumTruePeakLevel);
    copyXmlToBinary (*xml, destData);
}

void LoudnessMeterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName ("LoudnessMeterParams"))
        {
            *targetIntegratedLoudness = (float) xmlState->getDoubleAttribute ("targetIntegratedLoudness", *targetIntegratedLoudness);
            *targetMaximumShortTermLoudness = (float) xmlState->getDoubleAttribute ("targetMaximumShortTermLoudness", *targetMaximumShortTermLoudness);
            *targetMaximumTruePeakLevel = (float) xmlState->getDoubleAttribute ("targetMaximumTruePeakLevel", *targetMaximumTruePeakLevel);
        }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LoudnessMeterAudioProcessor();
}
