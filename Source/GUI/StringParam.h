/*
  ==============================================================================

    String.h
    Created: 26 Aug 2024 11:18:01pm
    Author:  a931e

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "Util.h"
using namespace juce;

struct SliderSpec {
	String name;
	String paramId;
};

class SliderGroup : public juce::GroupComponent
{

public:
	SliderGroup(PhysicsBasedSynthAudioProcessor& p, String name,std::vector<SliderSpec> sliderSpecs) : juce::GroupComponent(name,name), audioProcessor(p)
	{
		for (SliderSpec spec : sliderSpecs)
		{
			auto slider = new SliderWithName(spec.name, audioProcessor, spec.paramId);
			addAndMakeVisible(slider);
			
			sliders.push_back(slider);
		}
	}

	~SliderGroup()
	{
		for (SliderWithName* slider : sliders)
		{
			delete slider;
		}
	}

	void resized() override
	{
		juce::FlexBox fb;
		fb.flexWrap = juce::FlexBox::Wrap::wrap;
		fb.alignItems = juce::FlexBox::AlignItems::stretch;
		fb.justifyContent = juce::FlexBox::JustifyContent::center;
		fb.alignContent = juce::FlexBox::AlignContent::stretch;

		
		for (SliderWithName* slider : sliders)
		{
			fb.items.add(juce::FlexItem(*slider).withMinWidth(80).withMinHeight(90).withMargin(20).withFlex(1));
		}

		fb.performLayout(getLocalBounds());
	}

private:
	PhysicsBasedSynthAudioProcessor& audioProcessor;

	std::vector<SliderWithName*> sliders;

};
