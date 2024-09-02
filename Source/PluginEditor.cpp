/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PhysicsBasedSynthAudioProcessorEditor::PhysicsBasedSynthAudioProcessorEditor (PhysicsBasedSynthAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), stringParamComponent(p, "String",{
		{"String Length 123", "string_length"},
		{"String Density", "string_density"},
		{"String Stiffness", "string_stiffness"},
		{"String Harmonics", "string_harmonics"},
		{"String Damping", "string_damping"},
		}),
	hammerParamComponent(p, "Hammer", {
		{"Hammer Mass", "hammer_mass"},
		{"Hammer Position", "hammer_position"},
		{"Hammer Velocity", "hammer_velocity"},
		{"Hammer Young's Modulus", "hammer_youngs_modulus"}
		})
{
	setSize(1000, 700);
	

	addAndMakeVisible(stringParamComponent);
	addAndMakeVisible(hammerParamComponent);
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

	juce::FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;

	fb.items.add(FlexItem(stringParamComponent).withFlex(1).withMargin(5));
	fb.items.add(FlexItem(hammerParamComponent).withFlex(1).withMargin(5));

	fb.performLayout(area);
}
