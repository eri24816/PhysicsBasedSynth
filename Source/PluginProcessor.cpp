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
        mySynth.addVoice(new SynthVoice());
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

    params.push_back(std::make_unique<AudioParameterFloat>("ATTACK", "Attack", 0.1f, 5000.0f, 0.1f));
    params.push_back(std::make_unique<AudioParameterFloat>("DECAY", "Decay", 1.0f, 2000.0f, 1.0f));
    params.push_back(std::make_unique<AudioParameterFloat>("SUSTAIN", "Sustain", 0.0f, 1.0f, 0.8f));
    params.push_back(std::make_unique<AudioParameterFloat>("RELEASE", "Release", 0.1f, 5000.0f, 0.1f));
    params.push_back(std::make_unique<AudioParameterInt>("WAVEFORM", "Waveform", 0, 2, 0));
    params.push_back(std::make_unique<AudioParameterFloat>("FILTER_CUTOFF", "FilterCutoff", 2.0f, 10000.0f, 400.0f));
    params.push_back(std::make_unique<AudioParameterFloat>("FILTER_RESONANCE", "FilterResonance", 1.0f, 5.0f, 1.0f));
    params.push_back(std::make_unique<AudioParameterInt>("FILTER_TYPE", "FilterType", 0, 2, 0));

    return { params.begin(), params.end() };
}