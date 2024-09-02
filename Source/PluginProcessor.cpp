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

    //for (int i = 0; i < mySynth.getNumVoices(); i++)
    //{
    //    if ((myVoice = dynamic_cast<SynthVoice*>(mySynth.getVoice(i))))
    //    {
    //        auto attackValue = valueTree.getRawParameterValue("ATTACK");
    //        auto decayValue = valueTree.getRawParameterValue("DECAY");
    //        auto sustainValue = valueTree.getRawParameterValue("SUSTAIN");
    //        auto releaseValue = valueTree.getRawParameterValue("RELEASE");

    //        myVoice->getEnvelope(attackValue->load(),
    //            decayValue->load(),
    //            sustainValue->load(),
    //            releaseValue->load());

    //        auto waveformValue = valueTree.getRawParameterValue("WAVEFORM");

    //        myVoice->getOscWaveform(waveformValue->load());

    //        auto filterTypeValue = valueTree.getRawParameterValue("FILTER_TYPE");
    //        auto filterCutoffValue = valueTree.getRawParameterValue("FILTER_CUTOFF");
    //        auto filterResonanceValue = valueTree.getRawParameterValue("FILTER_RESONANCE");

    //        myVoice->getFilter(filterTypeValue->load(), filterCutoffValue->load(), filterResonanceValue->load());
    //    }
    //}

    buffer.clear();
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
	params.push_back(std::make_unique<AudioParameterFloat>("gain", "Gain", 0.0f, 1.0f, 0.5f));
	
	// string parameters
	// length, density, stiffness, damping, number of harmonics (tension is derived from these)
	params.push_back(std::make_unique<AudioParameterFloat>("string_length", "String Length", 0.2, 1.0, 0.5));
	params.push_back(std::make_unique<AudioParameterFloat>("string_density", "String Density", 0.01, 0.1, 0.03));
	params.push_back(std::make_unique<AudioParameterFloat>("string_stiffness", "String Stiffness", 0, 1.0,0.1));
	params.push_back(std::make_unique<AudioParameterFloat>("string_damping", "String Damping", 0.1f, 10.0f, 1.0f));
	params.push_back(std::make_unique<AudioParameterInt>("string_harmonics", "String Harmonics", 0,50,10));

	// hammer parameters
	// mass, position, velocity
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_mass", "Hammer Mass", 0.01f, 0.2f, 0.05f));
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_position", "Hammer Position", 0, 1.0f, 0.1));
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_velocity", "Hammer Velocity", 0.1f, 10.0f, 3));
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_youngs_modulus", "Hammer Young's Modulus", 500000, 8000000, 2000000));

    return { params.begin(), params.end() };
}