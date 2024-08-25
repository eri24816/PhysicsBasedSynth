/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PhysicsBasedSynthAudioProcessorEditor::PhysicsBasedSynthAudioProcessorEditor (PhysicsBasedSynthAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), oscGui(p), envGui(p), filterGui(p)
{
    setSize(600, 200);
	addAndMakeVisible(oscGui);
	addAndMakeVisible(envGui);
	addAndMakeVisible(filterGui);
}

PhysicsBasedSynthAudioProcessorEditor::~PhysicsBasedSynthAudioProcessorEditor()
{
}

//==============================================================================
void PhysicsBasedSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void PhysicsBasedSynthAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    Rectangle<int> area = getLocalBounds();

    const int componentWidth = 200;
    const int componentHeight = 200;

    oscGui.setBounds(area.removeFromLeft(componentWidth).removeFromTop(componentHeight));
    envGui.setBounds(area.removeFromLeft(componentWidth).removeFromTop(componentHeight));
    filterGui.setBounds(area.removeFromLeft(componentWidth).removeFromTop(componentHeight));

}
