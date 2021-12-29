/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Vendor/Ebu128LoudnessMeter/Ebu128LoudnessMeter.h"
#include "Vendor/TruePeakProcessor/TruePeakProcessor.h"


//==============================================================================
/**
*/
class LoudnessMeterAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    LoudnessMeterAudioProcessor();
    ~LoudnessMeterAudioProcessor() override;
    
    Ebu128LoudnessMeter loudnessMeter;

    juce::WindowedSincInterpolator interpolator;
    juce::AudioBuffer<float> interpolatedBuffer;
    
    AudioProcessing::TruePeak truePeakProcessor;
   
    juce::AudioParameterFloat*  targetIntegratedLoudness;
    juce::AudioParameterFloat*  targetMaximumShortTermLoudness;
    juce::AudioParameterFloat*  targetMaximumTruePeakLevel;
    
    float integratedLoudness =          -INFINITY;
    float shortTermLoudness =           -INFINITY;
    float momentaryLoudness =           -INFINITY;
    float maximumShortTermLoudness =    -INFINITY;
    float maximumMomentaryLoudness =    -INFINITY;
    float loudnessRange =               0;
    float peakLevel =                   -INFINITY;
    float maximumPeakLevel =            -INFINITY;
    float truePeakLevel =               -INFINITY;
    float maximumTruePeakLevel =        -INFINITY;
    
    bool measurementPaused = true;
    
    const int oversampling = 2;
    
    void resetIntegratedLoudness();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    

private:
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoudnessMeterAudioProcessor)
};
