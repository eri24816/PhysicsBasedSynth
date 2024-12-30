/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "data.h"

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

    for (int i = 0; i < 10; i++)
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

    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    convolution.reset(); 
    convolution.prepare(spec); 
	convolution.loadImpulseResponse(
        piano_ir,
        sizeof(char) * piano_ir_len,
        juce::dsp::Convolution::Stereo::yes,
		juce::dsp::Convolution::Trim::no,
        0
    );
    dryBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
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
    // dry signal
    
    dryBuffer.makeCopyOf(buffer, true);
    dsp::AudioBlock<float> block(buffer);
    dsp::ProcessContextReplacing<float> context(block);
    convolution.process(context);

    float wet = valueTree.getRawParameterValue("wet_dry")->load();
	float dry = 1 - wet;

	// gain wet signal
	context.getOutputBlock().multiplyBy(wet);

	// gain dry signal
	dryBuffer.applyGain(dry);

	// add dry signal
	context.getOutputBlock().add<float>(dryBuffer);

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
	auto state = valueTree.copyState();
	std::unique_ptr<XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void PhysicsBasedSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(valueTree.state.getType()))
            valueTree.replaceState(juce::ValueTree::fromXml(*xmlState));
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
	params.push_back(std::make_unique<AudioParameterFloat>("gain", "Gain", 0.0f, 5.0f, 1));
	params.push_back(std::make_unique<AudioParameterFloat>("wet_dry", "Wet Dry", 0.0f, 1.0f, 0.5f));
	
	// string parameters
	// length, density, stiffness, damping, number of harmonics (tension is derived from these)
	params.push_back(std::make_unique<AudioParameterFloat>("string_length", "String Length", 0.3, 10.0, 1));
	params.push_back(std::make_unique<AudioParameterFloat>("string_density", "String Density", 0.5, 10.0, 1));
	params.push_back(std::make_unique<AudioParameterFloat>("string_stiffness", "String Stiffness", 0, 5000.0, 1)); 
	params.push_back(std::make_unique<AudioParameterFloat>("string_damping", "String Damping", 0, 20.0, 1));

	params.push_back(std::make_unique<AudioParameterInt>("string_harmonics", "String Harmonics", 8,48,48));

	// hammer parameters
	// mass, position, velocity
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_mass", "Hammer Mass (g)", 0.1, 20, 9));
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_position", "Hammer Position", 0, 1.0f, 0.1));
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_velocity", "Hammer Velocity", 0.1f, 10.0f, 3));
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_hardness", "Hammer Hardness", 0.01f, 150, 1));
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_nonlinearity", "Hammer Nonlinearity", 1, 5, 3.7));
	params.push_back(std::make_unique<AudioParameterFloat>("hammer_width", "Hammer Width", 5, 50, 10));

    // visualizing parameters
	// x scale, y scale, time scale, note
	params.push_back(std::make_unique<AudioParameterFloat>("visualizer_x_scale", "Visualizer X Scale", 0.1, 10.0, 1.0));
	params.push_back(std::make_unique<AudioParameterFloat>("visualizer_y_scale", "Visualizer Y Scale", 1, 500.0, 100.0));
	params.push_back(std::make_unique<AudioParameterFloat>("visualizer_time_scale", "Visualizer Time Scale", NormalisableRange<float>(0.0001, 1.0,0), 0.0001));
	params.push_back(std::make_unique<AudioParameterInt>("visualizer_note", "Visualizer Note", 0, 127, 60));
    return { params.begin(), params.end() };
}
