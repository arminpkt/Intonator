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
    juce::ValueTree root("STATE");

    {
        const juce::ScopedLock sl(gridStateLock);
        root.addChild(GridStateSerialiser::toValueTree(gridState), -1, nullptr);
    }

    {
        const juce::ScopedLock sl(pianoRollStateLock);
        root.addChild(PianoRollStateSerialiser::toValueTree(pianoRollState), -1, nullptr);
    }

    if (auto xml = root.createXml())
        copyXmlToBinary(*xml, destData);
}

void UnTETeredAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (!xml)
        return;

    const auto root = juce::ValueTree::fromXml(*xml);
    if (!root.isValid())
        return;

    const auto gridTree = root.getChildWithName(GridStateSerialiser::treeType());
    if (!gridTree.isValid())
        return;

    setGridState(GridStateSerialiser::fromValueTree(gridTree));
}

GridState UnTETeredAudioProcessor::getGridState() const
{
    const juce::ScopedLock sl(gridStateLock);
    return gridState;
}

void UnTETeredAudioProcessor::setGridState(const GridState& s)
{
    const juce::ScopedLock sl(gridStateLock);
    gridState = s;
}

void UnTETeredAudioProcessor::updateGridState(std::function<void(GridState&)> fn)
{
    const juce::ScopedLock sl(gridStateLock);
    fn(gridState);
}

PianoRollState UnTETeredAudioProcessor::getPianoRollState() const
{
    const juce::ScopedLock sl(pianoRollStateLock);
    return pianoRollState;
}

void UnTETeredAudioProcessor::setPianoRollState(const PianoRollState& s)
{
    const juce::ScopedLock sl(pianoRollStateLock);
    pianoRollState = s;
}

void UnTETeredAudioProcessor::updatePianoRollState(std::function<void(PianoRollState&)> fn)
{
    const juce::ScopedLock sl(pianoRollStateLock);
    fn(pianoRollState);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UnTETeredAudioProcessor();
}
