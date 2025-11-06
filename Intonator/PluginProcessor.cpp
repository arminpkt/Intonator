#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
    DBG("TEST");
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    DBG("PREPARE TO PLAY");

    mergedMidiSequence.clear();
    currentSamplePosition = 0.0;
    currentEventIndex = 0;

    // Fixed tempo assumption
    constexpr double bpm = 120.0;
    constexpr double secondsPerBeat = 60.0 / bpm;
    constexpr double measureLength = 4.0 * secondsPerBeat; // One 4/4 measure

    constexpr double noteOnTime = 0.0;
    constexpr double noteOffTime = measureLength;

    // MIDI note numbers
    constexpr int noteA3 = 57;       // A3 = 220 Hz
    constexpr int noteCSharp4 = 61;  // C#4 = ~277.18 Hz, need to bend it to 275 Hz

    // Channels
    constexpr int channelA3 = 2;
    constexpr int channelCSharp4 = 3;

    // Pitch bend: C#4 down to 275 Hz (Just Intonation 5:4 from A3)
    constexpr double bendInSemitones = -0.137; // Approx. 13.7 cents down
    constexpr double bendRange = 2.0; // ±2 semitones assumed
    int bendValue = static_cast<int>(8192 + (bendInSemitones / bendRange) * 8192);
    bendValue = std::clamp(bendValue, 0, 16383);

    // Add pitch bend for C#4 channel
    mergedMidiSequence.addEvent(juce::MidiMessage::pitchWheel(channelCSharp4, bendValue), noteOnTime);

    // Note-on
    mergedMidiSequence.addEvent(juce::MidiMessage::noteOn(channelA3, noteA3, static_cast<juce::uint8>(127)), noteOnTime);
    mergedMidiSequence.addEvent(juce::MidiMessage::noteOn(channelCSharp4, noteCSharp4, static_cast<juce::uint8>(127)), noteOnTime);

    // Note-off
    mergedMidiSequence.addEvent(juce::MidiMessage::noteOff(channelA3, noteA3), noteOffTime);
    mergedMidiSequence.addEvent(juce::MidiMessage::noteOff(channelCSharp4, noteCSharp4), noteOffTime);

    mergedMidiSequence.updateMatchedPairs();
    mergedMidiSequence.sort();

    DBG("Merged sequence has " << mergedMidiSequence.getNumEvents() << " events");
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    //DBG("PROCESS BLOCK");

    auto numSamples = buffer.getNumSamples();
    juce::MidiBuffer tempBuffer;

    juce::AudioPlayHead* playHead = getPlayHead();
    double hostPositionInSamples = currentSamplePosition;
    bool hostIsPlaying = true;

    if (playHead != nullptr)
    {
        juce::AudioPlayHead::CurrentPositionInfo posInfo;
        if (playHead->getCurrentPosition(posInfo))
        {
            hostIsPlaying = posInfo.isPlaying;
            hostPositionInSamples = posInfo.timeInSeconds * getSampleRate();
        }
    }

    // Detect jumps or scrubbing
    if (std::abs(hostPositionInSamples - lastHostPosition) > getSampleRate() * 0.01)
    {
        for (auto& [channel, note] : activeNotes)
            tempBuffer.addEvent(juce::MidiMessage::noteOff(channel, note), 0);

        activeNotes.clear();
        currentSamplePosition = hostPositionInSamples;

        // Reset event index to first event after new host position
        currentEventIndex = 0;
        while (currentEventIndex < mergedMidiSequence.getNumEvents() &&
               mergedMidiSequence.getEventTime(currentEventIndex) * getSampleRate() < currentSamplePosition)
        {
            ++currentEventIndex;
        }
    }

    lastHostPosition = hostPositionInSamples;

    if (mergedMidiSequence.getNumEvents() > 0 && hostIsPlaying)
    {
        // Send MIDI events from currentEventIndex only
        while (currentEventIndex < mergedMidiSequence.getNumEvents())
        {
            auto& e = mergedMidiSequence.getEventPointer(currentEventIndex)->message;
            double eventTimeInSamples = mergedMidiSequence.getEventTime(currentEventIndex) * getSampleRate();

            if (eventTimeInSamples >= currentSamplePosition + numSamples)
                break; // Stop for this block

            if (eventTimeInSamples >= currentSamplePosition)
            {
                int sampleOffset = static_cast<int>(eventTimeInSamples - currentSamplePosition);
                tempBuffer.addEvent(e, sampleOffset);

                DBG(e.getDescription());

                if (e.isNoteOn())
                    activeNotes.insert({e.getChannel(), e.getNoteNumber()});

                else if (e.isNoteOff())
                    activeNotes.erase({e.getChannel(), e.getNoteNumber()});
            }

            ++currentEventIndex;
        }

        currentSamplePosition += numSamples;
    }
    else if (!hostIsPlaying)
    {
        for (auto& [channel, note] : activeNotes)
            tempBuffer.addEvent(juce::MidiMessage::noteOff(channel, note), 0);

        activeNotes.clear();
    }

    double lastEventTime = mergedMidiSequence.getEndTime() * getSampleRate();
    if (currentSamplePosition >= lastEventTime)
    {
        for (auto& [channel, note] : activeNotes)
            tempBuffer.addEvent(juce::MidiMessage::noteOff(channel, note), 0);

        activeNotes.clear();
        currentSamplePosition = lastEventTime;
    }

    midiMessages.addEvents(tempBuffer, 0, numSamples, 0);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
