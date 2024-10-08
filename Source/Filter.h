/*
  ==============================================================================

    Filter.h
    Created: 23 Oct 2020 6:22:17am
    Author:  will

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

using namespace juce;


class Filter  : public Component
{
public:
    Filter(PhysicsBasedSynthAudioProcessor&);
    ~Filter() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PhysicsBasedSynthAudioProcessor& processor;

    Slider filterCutoff;
    Slider filterResonance;
    ComboBox filterSelector;

    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> filterCutoffAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> filterResonanceAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> filterSelectorAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Filter)
};
