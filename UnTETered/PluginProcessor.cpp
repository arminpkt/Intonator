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
const juce::String UnTETeredAudioProcessor::getName() const { return JucePlugin_Name; }

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

double UnTETeredAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int    UnTETeredAudioProcessor::getNumPrograms()              { return 1; }
int    UnTETeredAudioProcessor::getCurrentProgram()           { return 0; }
void   UnTETeredAudioProcessor::setCurrentProgram(int index)  { juce::ignoreUnused(index); }

const juce::String UnTETeredAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void UnTETeredAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void UnTETeredAudioProcessor::prepareToPlay(double /*sampleRate*/, int /*samplesPerBlock*/) {}
void UnTETeredAudioProcessor::releaseResources() {}

bool UnTETeredAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif
    return true;
  #endif
}

void UnTETeredAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    midiMessages.clear();

    {
        const juce::ScopedLock sl(previewMessagesLock);
        for (const auto& msg : pendingPreviewMessages)
            midiMessages.addEvent(msg, 0);
        pendingPreviewMessages.clear();
    }

    auto* playHead = getPlayHead();
    if (playHead == nullptr)
        return;

    setTransportStateFromHost(playHead);
    playMidi(midiMessages, buffer.getNumSamples());
}

//==============================================================================
bool UnTETeredAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* UnTETeredAudioProcessor::createEditor()
{
    return new UnTETeredAudioProcessorEditor(*this);
}

//==============================================================================
void UnTETeredAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
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

void UnTETeredAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (!xml) return;

    const auto root = juce::ValueTree::fromXml(*xml);
    if (!root.isValid()) return;

    const auto gridTree = root.getChildWithName(GridStateSerialiser::treeType());
    if (gridTree.isValid())
        setGridState(GridStateSerialiser::fromValueTree(gridTree));

    const auto pianoRollTree = root.getChildWithName(PianoRollStateSerialiser::treeType());
    if (pianoRollTree.isValid())
        setPianoRollState(PianoRollStateSerialiser::fromValueTree(pianoRollTree));
}

//==============================================================================
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
    playbackDirty.store(true, std::memory_order_release);
}

void UnTETeredAudioProcessor::setPianoRollViewState(float octaveHeightPxF,
                                                     float barWidthPxF,
                                                     double freqBottomScreen,
                                                     float barLeftScreen)
{
    const juce::ScopedLock sl(pianoRollStateLock);
    pianoRollState.octaveHeightPxF  = octaveHeightPxF;
    pianoRollState.barWidthPxF      = barWidthPxF;
    pianoRollState.freqBottomScreen = freqBottomScreen;
    pianoRollState.barLeftScreen    = barLeftScreen;
}

void UnTETeredAudioProcessor::updatePianoRollState(std::function<void(PianoRollState&)> fn)
{
    const juce::ScopedLock sl(pianoRollStateLock);
    fn(pianoRollState);
}

void UnTETeredAudioProcessor::addPreviewMessages(std::initializer_list<juce::MidiMessage> msgs)
{
    const juce::ScopedLock sl(previewMessagesLock);
    for (const auto& msg : msgs)
        pendingPreviewMessages.push_back(msg);
}

//==============================================================================
const TransportState& UnTETeredAudioProcessor::getTransportState() { return transportState; }

void UnTETeredAudioProcessor::setTransportStateFromHost(juce::AudioPlayHead* ph)
{
    if (!ph) return;
    if (auto pos = ph->getPosition()) {
        transportState.isPlaying.store(pos->getIsPlaying(), std::memory_order_relaxed);
        transportState.isRecording.store(pos->getIsRecording(), std::memory_order_relaxed);
        if (auto bpm     = pos->getBpm())             transportState.bpm.store(*bpm, std::memory_order_relaxed);
        if (auto ppq     = pos->getPpqPosition())     transportState.ppqPosition.store(*ppq, std::memory_order_relaxed);
        if (auto samples = pos->getTimeInSamples())   transportState.timeInSamples.store(*samples, std::memory_order_relaxed);
        if (auto sig     = pos->getTimeSignature()) {
            transportState.numerator.store(sig->numerator, std::memory_order_relaxed);
            transportState.denominator.store(sig->denominator, std::memory_order_relaxed);
        }
    }
    transportState.sampleRate.store(getSampleRate(), std::memory_order_relaxed);
}

void UnTETeredAudioProcessor::rebuildPlaybackSequence()
{
    PianoRollState stateCopy;
    {
        const juce::ScopedLock sl(pianoRollStateLock);
        stateCopy = pianoRollState;
    }
    auto region = makeNoteRegionFromState(stateCopy, PITCH_BEND_RANGE);
    PlaybackSequence newSeq;
    newSeq.messages      = region.midiMessages;
    newSeq.pitchBendRange = PITCH_BEND_RANGE;
    newSeq.valid          = true;
    {
        const juce::ScopedLock sl(playbackLock);
        playbackSequence = std::move(newSeq);
    }
    playbackDirty.store(false, std::memory_order_release);
}

void UnTETeredAudioProcessor::flushActiveNotes(juce::MidiBuffer& midiMessages)
{
    for (const auto& [channel, noteNumber] : activeNotes)
        midiMessages.addEvent(juce::MidiMessage::noteOff(channel, noteNumber), 0);
    activeNotes.clear();
}

void UnTETeredAudioProcessor::playMidi(juce::MidiBuffer& midiMessages, int numSamples)
{
    if (playbackDirty.load(std::memory_order_acquire))
        rebuildPlaybackSequence();

    const bool   isPlaying   = transportState.isPlaying.load(std::memory_order_relaxed);
    const double bpm         = transportState.bpm.load(std::memory_order_relaxed);
    const double ppq         = transportState.ppqPosition.load(std::memory_order_relaxed);
    const double sr          = transportState.sampleRate.load(std::memory_order_relaxed);
    const int    numerator   = transportState.numerator.load(std::memory_order_relaxed);
    const int    denominator = transportState.denominator.load(std::memory_order_relaxed);

    if (!isPlaying || bpm <= 0.0 || sr <= 0.0 || denominator <= 0 || numSamples <= 0)
    {
        flushActiveNotes(midiMessages);
        lastExpectedBar = -1.0;
        return;
    }

    const double qnPerBar = 4.0 * static_cast<double>(numerator) / static_cast<double>(denominator);
    const double startBar = ppq / qnPerBar;
    const double blockSeconds      = static_cast<double>(numSamples) / sr;
    const double blockQuarterNotes = blockSeconds / (60.0 / bpm);
    const double blockBars         = blockQuarterNotes / qnPerBar;
    const double endBar            = startBar + blockBars;

    const bool positionJumpedBack = (lastExpectedBar >= 0.0)
                                 && (startBar < lastExpectedBar - 0.5 * blockBars);
    if (positionJumpedBack)
        flushActiveNotes(midiMessages);

    lastExpectedBar = endBar;

    PlaybackSequence seqCopy;
    {
        const juce::ScopedLock sl(playbackLock);
        seqCopy = playbackSequence;
    }
    if (!seqCopy.valid) return;

    for (const auto& msg : seqCopy.messages)
    {
        const double eventBar = msg.getTimeStamp();
        if (eventBar < startBar || eventBar >= endBar) continue;

        const double relBars = eventBar - startBar;
        const int sampleOffset = juce::jlimit(
            0, numSamples - 1,
            static_cast<int>(std::floor(relBars * qnPerBar * (60.0 / bpm) * sr)));

        midiMessages.addEvent(msg, sampleOffset);

        if (msg.isNoteOn())  activeNotes.insert({ msg.getChannel(), msg.getNoteNumber() });
        else if (msg.isNoteOff()) activeNotes.erase({ msg.getChannel(), msg.getNoteNumber() });
    }
}

void UnTETeredAudioProcessor::requestHostSeekToBar(double targetBar) noexcept
{
    hostSeekRequest.targetBar.store(targetBar, std::memory_order_relaxed);
    hostSeekRequest.pending.store(true,        std::memory_order_relaxed);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UnTETeredAudioProcessor();
}