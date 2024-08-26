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

        string = std::make_shared<InstrumentPhysics::String>(frequency);
        hammer = std::make_shared<InstrumentPhysics::Rigidbody>(1.0f, 0.5f);
		simulation->addObject(string);
		simulation->addObject(hammer);
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
		const float dt = 1.0f / 44100.0f;
        for (int sample = 0; sample < numSamples; ++sample)
        {
			simulation->update(dt);
			const float currentSample = string->sampleU(0.01,simulation->getTime());
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
