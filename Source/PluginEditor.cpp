/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PhysicsBasedSynthAudioProcessorEditor::PhysicsBasedSynthAudioProcessorEditor (PhysicsBasedSynthAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), 
	mainParamComponent(p, "Main", {
		{"Gain 6", "gain"},
		}),
	stringParamComponent(p, "String",{
		{"String Length", "string_length"},
		{"String Density", "string_density"},
		{"String Stiffness", "string_stiffness"},
		{"String Harmonics", "string_harmonics"},
		{"String Damping", "string_damping"},
		}),
	hammerParamComponent(p, "Hammer", {
		{"Hammer Mass (g)", "hammer_mass"},
		{"Hammer Position", "hammer_position"},
		{"Hammer Velocity", "hammer_velocity"},
		{"Hammer Young's Modulus", "hammer_youngs_modulus"}
		}),
	visualParamComponent(p, "Visual", {
		{"X Scale", "visualizer_x_scale", 0.5},
		{"Y Scale", "visualizer_y_scale", 0.5},
		{"Time Scale", "visualizer_time_scale", 0.2},
		{"Note", "visualizer_note"}
		}),
	visualizer(p.valueTree)
{
	setSize(1000, 700);

	stringParamComponent.getSlider("string_harmonics")->slider.setNormalisableRange(NormalisableRange<double>(8, 48, 8));
	
	addAndMakeVisible(mainParamComponent);
	addAndMakeVisible(stringParamComponent);
	addAndMakeVisible(hammerParamComponent);
	addAndMakeVisible(visualParamComponent);
	addAndMakeVisible(visualizer);
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
	fb.flexDirection = FlexBox::Direction::row;

	fb.items.add(FlexItem(mainParamComponent).withFlex(1).withMargin(5));
	fb.items.add(FlexItem(stringParamComponent).withFlex(1).withMargin(5));
	fb.items.add(FlexItem(hammerParamComponent).withFlex(1).withMargin(5));

	fb.performLayout(area.removeFromTop(area.getHeight() * 0.7));

	visualParamComponent.setBounds(area.removeFromLeft(area.getWidth() * 0.3));
	visualizer.setBounds(area.reduced(10));
}
