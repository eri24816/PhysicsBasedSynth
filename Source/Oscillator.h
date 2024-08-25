/**
 * @file Oscillator.h
 * 
 * @brief 
 * 
 * @author
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

using namespace juce;


class Oscillator : public Component
{
public:
    Oscillator(PhysicsBasedSynthAudioProcessor&);
    ~Oscillator();

    void paint (Graphics&) override;
    void resized() override;

private:
    ComboBox wavefromSelector;

    std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment> waveformSelectorAttachment;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PhysicsBasedSynthAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oscillator)
};
