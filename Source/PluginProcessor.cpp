/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;


//==============================================================================
PhysicsBasedSynthAudioProcessor::PhysicsBasedSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif

    ,valueTree(*this, nullptr, "Parameters", createParameters())
{
    mySynth.clearVoices();

    for (int i = 0; i < 5; i++)
    {
        auto voice = new SynthVoice();
        voice->setValueTree(valueTree);
        mySynth.addVoice(voice);
    }


    mySynth.clearSounds();
    mySynth.addSound(new SynthSound());
}

PhysicsBasedSynthAudioProcessor::~PhysicsBasedSynthAudioProcessor()
{
}

//==============================================================================
const juce::String PhysicsBasedSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhysicsBasedSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PhysicsBasedSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PhysicsBasedSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PhysicsBasedSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhysicsBasedSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PhysicsBasedSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PhysicsBasedSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PhysicsBasedSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void PhysicsBasedSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PhysicsBasedSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    mySynth.setCurrentPlaybackSampleRate(sampleRate);
}

void PhysicsBasedSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PhysicsBasedSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PhysicsBasedSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    mySynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool PhysicsBasedSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PhysicsBasedSynthAudioProcessor::createEditor()
{
    return new PhysicsBasedSynthAudioProcessorEditor (*this);
}

//==============================================================================
void PhysicsBasedSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PhysicsBasedSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhysicsBasedSynthAudioProcessor();
}

AudioProcessorValueTreeState::ParameterLayout PhysicsBasedSynthAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

	// general parameters
    // gain
	params.push_back(std::make_unique<AudioParameterFloat>("gain", "Gain", 0.0f, 2.0f, 1));
	
	// string parameters
	// length, density, stiffness, damping, number of harmonics (tension is derived from these)
	params.push_back(std::make_unique<AudioParameterFloat>("string_length", "String Length", 0.2, 1.0, 0.5));
	params.push_back(std::make_unique<AudioParameterFloat>("string_density", "String Density", 0.01, 0.1, 0.03));
	params.push_back(std::make_unique<AudioParameterFloat>("string_stiffness", "String Stiffness", 0, 1.0,0.1));
	params.push_back(std::make_unique<AudioParameterFloat>("string_damping", "String Damping", 0.1f, 10.0f, 1.0f));
	params.push_back(std::make_unique<AudioParameterInt>("string_harmonics", "String Harmonics", 0,50,10));

	// hammer parameters
	// mass, position, velocity
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_mass", "Hammer Mass (g)", 0.1, 5, 1));
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_position", "Hammer Position", 0, 1.0f, 0.1));
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_velocity", "Hammer Velocity", 0.1f, 10.0f, 3));
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_youngs_modulus", "Hammer Young's Modulus", 200000, 8000000, 2000000));

    // visualizing parameters
	// x scale, y scale, time scale
	params.push_back(std::make_unique<AudioParameterFloat>("visualizer_x_scale", "Visualizer X Scale", 0.1, 10.0, 1.0));
	params.push_back(std::make_unique<AudioParameterFloat>("visualizer_y_scale", "Visualizer Y Scale", 0.1, 10.0, 1.0));
	params.push_back(std::make_unique<AudioParameterFloat>("visualizer_time_scale", "Visualizer Time Scale", 0.001, 1.0, 0.001));

    return { params.begin(), params.end() };
}