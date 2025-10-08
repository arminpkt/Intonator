#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
UnTETeredAudioProcessor::UnTETeredAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
}

UnTETeredAudioProcessor::~UnTETeredAudioProcessor()
{
}

//==============================================================================
const juce::String UnTETeredAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool UnTETeredAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool UnTETeredAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool UnTETeredAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double UnTETeredAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int UnTETeredAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int UnTETeredAudioProcessor::getCurrentProgram()
{
    return 0;
}

void UnTETeredAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String UnTETeredAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void UnTETeredAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void UnTETeredAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void UnTETeredAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool UnTETeredAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void UnTETeredAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    if (auto* playHead = getPlayHead()) {
        setTransportStateFromHost(playHead);
        //playMidi();
    }

    for (const auto& midi : midiBuffer)
        DBG(midi.getMessage().getDescription());
    std::swap(midiBuffer, midiMessages);
}

//==============================================================================
bool UnTETeredAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* UnTETeredAudioProcessor::createEditor()
{
    return new UnTETeredAudioProcessorEditor (*this);
}

//==============================================================================
void UnTETeredAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void UnTETeredAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

const TransportState& UnTETeredAudioProcessor::getTransportState() { return transportState; }

void UnTETeredAudioProcessor::setTransportStateFromHost(juce::AudioPlayHead* playHead) {
    if (playHead) {
        if (auto pos = playHead->getPosition()) {
            transportState.isPlaying.store(pos->getIsPlaying(), std::memory_order_relaxed);
            transportState.isRecording.store(pos->getIsRecording(), std::memory_order_relaxed);

            if (auto bpm = pos->getBpm())
                transportState.bpm.store(*bpm, std::memory_order_relaxed);
            if (auto ppq = pos->getPpqPosition())
                transportState.ppqPosition.store(*ppq, std::memory_order_relaxed);
            if (auto samples = pos->getTimeInSamples())
                transportState.timeInSamples.store(*samples, std::memory_order_relaxed);

            if (auto sig = pos->getTimeSignature()) {
                transportState.numerator.store(sig->numerator, std::memory_order_relaxed);
                transportState.denominator.store(sig->denominator, std::memory_order_relaxed);
            }
        }

        transportState.sampleRate.store(getSampleRate(), std::memory_order_relaxed);
    }
}

void UnTETeredAudioProcessor::requestHostSeekToBar(double targetBar) noexcept {
    hostSeekRequest.targetBar.store(targetBar, std::memory_order_relaxed);
    hostSeekRequest.pending.store(true , std::memory_order_relaxed);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UnTETeredAudioProcessor();
}
