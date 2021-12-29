/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class LoudnessMeterAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    LoudnessMeterAudioProcessorEditor (LoudnessMeterAudioProcessor&);
    ~LoudnessMeterAudioProcessorEditor() override;
    
    int IntegratedLoudnessPrecision = 0;
    int maximumShortTermLoudnessPrecision = 0;
    int maximumTruePeakLevelPrecision = 1;
    
    bool extended;
    
    void drawBarGraph(juce::Graphics& g, int x, int y, int w, int minValue, int maxValue, float level, float target, juce::String valueLabel, juce::String warningLabel, bool over);
    
    const juce::Colour black = juce::Colour(0xff000000);
    const juce::Colour white = juce::Colour(0xffffffff);
    const juce::Colour green = juce::Colour(0xff29c732);
    const juce::Colour red = juce::Colour(0xfffc2125);
    const juce::Colour yellow = juce::Colour(0xfffec309);
    const juce::Colour grey = juce::Colour(0xff575757);
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LoudnessMeterAudioProcessor& audioProcessor;
    
    juce::TextButton resetButton { "Reset" };
    juce::TextButton presetButton { "Target" };
    juce::TextButton setButton { "Set" };
    
    
    juce::Slider integratedLoudnessSlider;
    juce::SliderParameterAttachment integratedLoudnessSliderAttachment;
    
    juce::Slider maximumShortTermLoudnessSlider;
    juce::SliderParameterAttachment maximumShortTermLoudnessSliderAttachment;
    
    juce::Slider maximumTruePeakLevelSlider;
    juce::SliderParameterAttachment maximumTruePeakLevelLoudnessSliderAttachment;

    void resetIntegratedLoudness();
    void choosePreset();
    void hide();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoudnessMeterAudioProcessorEditor)
};
