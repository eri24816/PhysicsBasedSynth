/**
 * @file SynthVoice.h
 * 
 * @brief 
 * 
 * @author
 */


#pragma once

#include <JuceHeader.h>
#include "SynthSound.h"

#include "Simulation.h"
#include "String.h"
#include "Rigidbody.h"
#include "PhysicsUtil.h"

using namespace juce;


class SynthVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast <SynthSound*>(sound) != nullptr;
    }
    
    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition) override
    {
		this->pitch = midiNoteNumber;
        this->velocity = velocity;
        frequency = MidiMessage::getMidiNoteInHertz(midiNoteNumber);

		simulation = std::make_unique<InstrumentPhysics::Simulation>();
		string = std::make_shared<InstrumentPhysics::String>(200.0 / frequency, 8000, 0.0324, 0, 10, 0.7);
		hammer = std::make_shared<InstrumentPhysics::Rigidbody>(0.05, InstrumentPhysics::Transform(0.1, 0.001));
		simulation->addObject(string);
		simulation->addObject(hammer);

		string->applyImpulse(0.1, 0, 0.1);
    }
    
    void stopNote (float velocity, bool allowTailOff) override
    {
		simulation.reset();
        if (velocity == 0)
            clearCurrentNote();
    }
    
    void pitchWheelMoved (int newPitchWheelValue) override
    {
        
    }
    
    void controllerMoved (int controllerNumber, int newControllerValue) override
    {
        
    }
    
    void renderNextBlock (AudioBuffer <float> &outputBuffer, int startSample, int numSamples) override
    {
		// fill 0 if there's no simulation
        if (!simulation) {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
                {
                    outputBuffer.addSample(channel, startSample, 0);
                }
                ++startSample;
            }
            return;
        }
		const float dt = 1.0f / 44100.0f;
        for (int sample = 0; sample < numSamples; ++sample)
        {
			simulation->update(dt);
            const float currentSample = string->sampleU(0.01, simulation->getTime()) * 100;
            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            {
				outputBuffer.addSample(channel, startSample, currentSample);
            }
            ++startSample;
        }
    }

private:
    int pitch;
    double velocity;
    double frequency;
    
	// objects are shared with simulation
    std::shared_ptr<InstrumentPhysics::String> string;
    std::shared_ptr<InstrumentPhysics::Rigidbody> hammer;

	std::unique_ptr<InstrumentPhysics::Simulation> simulation;

};
