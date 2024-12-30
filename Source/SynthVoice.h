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

        simulationDtCoarse = 1.0f / this->getSampleRate();
        simulationDtFine = simulationDtCoarse / 4.0;
		usingFineGrainSimulation = true;
		simulation = std::make_unique<InstrumentPhysics::Simulation>(simulationDtFine);


		string = std::make_shared<InstrumentPhysics::String>(stringProfile->getProfile(midiNoteNumber, stringLength, stringDensity, stringStiffness, stringDamping, stringHarmonics));
		hammer = std::make_shared<InstrumentPhysics::Rigidbody>(hammer_mass,
            InstrumentPhysics::Transform(getParam("hammer_position"),
                -0.001));
		simulation->addObject(string);
		simulation->addObject(hammer);

		// hammer compresses about 3e-4 m in average. This correction is added to avoid nonlinearity affect the magnitude of the force dramatically.
		const float hammerHardnessCorrection = pow(3e-3, -getParam("hammer_nonlinearity")); 

        simulation->addInteraction(std::make_shared<InstrumentPhysics::HammerStringInteraction>(
            hammer,
            InstrumentPhysics::Vector2<float>{0, 0},
            string, 
            30284 * hammerHardnessCorrection * getParam("hammer_hardness")*exp(0.045*(midiNoteNumber-21)),
			getParam("hammer_nonlinearity") + 0.015*(midiNoteNumber - 21),
			getParam("hammer_width") / 1000.0f // mm to m
        ));

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

		// do nothing if there's no simulation
        if (!simulation) {
            return;
        }
		const float gain = getParam("gain");
        for (int sample = 0; sample < numSamples; ++sample)
        {
			if (usingFineGrainSimulation)
			{
				simulation->update();
				simulation->update();
                simulation->update();
                simulation->update();
				if (simulation->getTime() > 5 * 0.001) // after 5ms, the hammer is considered to have left the string, so we can switch to coarse simulation
				{
					usingFineGrainSimulation = false;
					simulation->setDt(simulationDtCoarse);
				}
			}
            else {
                simulation->update();
            }
            const float currentSample = string->sampleU(0.01) * 875 * sqrt(string->getDensity()) * gain;
            //const float currentSample = string->sampleU(0.01) * 50 * gain;
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
	float simulationDtCoarse, simulationDtFine;
	bool usingFineGrainSimulation = false;

	std::unique_ptr<InstrumentPhysics::StringProfile> stringProfile = std::make_unique<InstrumentPhysics::GrandPianoStringProfile>();


	std::unique_ptr<InstrumentPhysics::Simulation> simulation;

	float getParam(String paramId)
	{
		return *valueTree->getRawParameterValue(paramId);
	}
};
