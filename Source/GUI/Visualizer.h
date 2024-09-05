/*
  ==============================================================================

	Visualizer.h
	Created: 4 Sep 2024 8:43:46pm
	Author:  a931e

  ==============================================================================
*/

#pragma once

#include <cmath>
#include "JuceHeader.h"
#include "../String.h"
#include "../Rigidbody.h"
#include "../SynthVoice.h"
using namespace juce;

constexpr int DUMMY_BUFFER_SIZE = 100;

class Visualizer : public Component, public Timer, public ValueTree::Listener
{

public:
	Visualizer(AudioProcessorValueTreeState& params) :
		dummyBuffer(1, DUMMY_BUFFER_SIZE),
		params(params)
	{
		startTimerHz(60);
		voiceForVisual.setValueTree(params);
		params.state.addListener(this);
		reset();
	}

	void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override {
		reset();
	}

	void reset() {
		voiceForVisual.startNote(60, 1, nullptr, 0);
		t = 0;
	}

	float toObjX(float x) {
		float xScale = params.getRawParameterValue("visualizer_x_scale")->load();
		return (x / (float)getWidth() - 0.5) * xScale + voiceForVisual.string->getLength() / 2;
	}

	float toScreenX(float x) {
		float xScale = params.getRawParameterValue("visualizer_x_scale")->load();
		return (x - voiceForVisual.string->getLength() / 2) / xScale * getWidth() + getWidth() / 2;
	}

	float toScreenY(float y) {
		float yScale = params.getRawParameterValue("visualizer_y_scale")->load();
		return getHeight() / 2 - getHeight() * yScale * y / 2;
	}

	float toObjY(float y) {
		float yScale = params.getRawParameterValue("visualizer_y_scale")->load();
		return 2 * (-y - getHeight() / 2) / getHeight() / yScale;
	}

	void paint(Graphics& g) override {
		float timeScale = params.getRawParameterValue("visualizer_time_scale")->load();
		float originalRate = voiceForVisual.getSampleRate();
		float dt = timeScale / 60.0f;
		int nSimulateStep = std::floor((t + dt) * originalRate) - std::floor(t * originalRate);

		nSimulateStep = std::min(nSimulateStep, DUMMY_BUFFER_SIZE);

		t += dt;
		voiceForVisual.renderNextBlock(dummyBuffer, 0, nSimulateStep);

		g.setColour(Colours::black);
		g.fillRoundedRectangle(getLocalBounds().toFloat(), 10);
		g.setColour(Colours::grey);
		g.drawRoundedRectangle(getLocalBounds().toFloat(), 10, 2);

		float xScale = params.getRawParameterValue("visualizer_x_scale")->load();
		float yScale = params.getRawParameterValue("visualizer_y_scale")->load();

		// draw string
		std::vector<std::shared_ptr<InstrumentPhysics::String>> strings{ voiceForVisual.string };
		for (auto& string : strings) {
			Path p;
			bool started = false;
			for (int i = 0; i < getWidth(); i++) {
				float x = toObjX(i);
				if (x < 0) continue;
				if (x > string->getLength()) break;
				
				float j = toScreenY(string->sampleU(x));
				if (!started) {
					p.startNewSubPath(i, j);
					started = true;
				}
				else {
					p.lineTo(i, j);
				}
			}
			g.strokePath(p, PathStrokeType(2));
		}

		// draw hammer

		auto hammer = voiceForVisual.hammer;
		float hammerX = toScreenX(hammer->transform.getWorldPos().x);
		float hammerY = toScreenY(hammer->transform.getWorldPos().y);
		g.setColour(Colours::red);
		g.fillEllipse(hammerX, hammerY, 10, 10);
		g.setColour(Colours::black);
		g.drawEllipse(hammerX, hammerY, 10, 10, 2);

	}

	void timerCallback() override
	{
		repaint();
	}

private:

	SynthVoice voiceForVisual;

	AudioBuffer<float> dummyBuffer;
	AudioProcessorValueTreeState& params;
	float t = 0;
};
