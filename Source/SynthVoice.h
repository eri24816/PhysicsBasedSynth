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

	void setValueTree(AudioProcessorValueTreeState& valueTree)
	{
		this->valueTree = &valueTree;
	}


    float calculateStringTension(float frequency, float density, float length, float stiffness)
    {
        float pi_stiffness_div_L = InstrumentPhysics::PI * stiffness / length;
        return 4 * frequency * frequency * density * length * length - pi_stiffness_div_L * pi_stiffness_div_L;
    }
    
    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition) override
    {
		this->pitch = midiNoteNumber;
        this->velocity = velocity;
        frequency = MidiMessage::getMidiNoteInHertz(midiNoteNumber);

        float stringLength = getParam("string_length");
		float stringDensity = getParam("string_density");
		float stringStiffness = getParam("string_stiffness");
		float stringDamping = getParam("string_damping");
		float stringHarmonics = getParam("string_harmonics");

		float hammer_mass = getParam("hammer_mass") * 0.001; // g to kg

		Logger::getCurrentLogger()->writeToLog("freq: " + String(frequency));


		simulation = std::make_unique<InstrumentPhysics::Simulation>();
		/*string = std::make_shared<InstrumentPhysics::String>(stringLength, stringTension, stringDensity,
            stringStiffness,
            (int)getParam("string_harmonics"),
			getParam("string_damping"));*/

		string = std::make_shared<InstrumentPhysics::String>(stringProfile->getProfile(midiNoteNumber, stringLength, stringDensity, stringStiffness, stringDamping, stringHarmonics));
		hammer = std::make_shared<InstrumentPhysics::Rigidbody>(hammer_mass,
            InstrumentPhysics::Transform(getParam("hammer_position"),
                -0.001));
		simulation->addObject(string);
		simulation->addObject(hammer);

        simulation->addInteraction(std::make_shared<InstrumentPhysics::HammerStringInteraction>(hammer, InstrumentPhysics::Vector2<float>{0, 0}, string, getParam(
            "hammer_youngs_modulus")));

		// give hammer a initial speed
		hammer->applyImpulse(InstrumentPhysics::Vector2<float>{0, 0}, InstrumentPhysics::Vector2<float>{0,hammer_mass * 
			getParam("hammer_velocity")});
    }
    
    void stopNote (float velocity, bool allowTailOff) override
    {
		simulation.reset();
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
		// clear buffer
        
		for (int i = 0; i < outputBuffer.getNumChannels(); i++)
		{
			outputBuffer.clear(i, startSample, numSamples);
		}


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
        const float dt = 1.0f / this->getSampleRate();
		const float gain = getParam("gain");
        for (int sample = 0; sample < numSamples; ++sample)
        {
			simulation->update(dt);
            const float currentSample = string->sampleU(0.01) * 50 * gain;
            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            {
				outputBuffer.addSample(channel, startSample, currentSample);
            }
            ++startSample;
        }
    }


    // objects are shared with simulation
    std::shared_ptr<InstrumentPhysics::String> string;
    std::shared_ptr<InstrumentPhysics::Rigidbody> hammer;

private:

	AudioProcessorValueTreeState* valueTree;

    int pitch;
    double velocity;
    double frequency;
    
	std::unique_ptr<InstrumentPhysics::StringProfile> stringProfile = std::make_unique<InstrumentPhysics::GrandPianoStringProfile>();


	std::unique_ptr<InstrumentPhysics::Simulation> simulation;

	float getParam(String paramId)
	{
		return *valueTree->getRawParameterValue(paramId);
	}
};
