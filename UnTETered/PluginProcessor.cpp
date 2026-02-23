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
    juce::ValueTree grid("GRID");

    const GridState s = getGridState();

    grid.setProperty("originHz", s.originFreqHz, nullptr);
    grid.setProperty("offsetX",  s.offsetX,     nullptr);
    grid.setProperty("offsetY",  s.offsetY,     nullptr);

    auto writePointSet = [](juce::ValueTree& parent, const char* name, const PointSet& set)
    {
        juce::ValueTree node(name);
        for (const auto& p : set)
        {
            juce::ValueTree c("C");
            c.setProperty("x", p.x, nullptr);
            c.setProperty("y", p.y, nullptr);
            node.addChild(c, -1, nullptr);
        }
        parent.addChild(node, -1, nullptr);
    };

    writePointSet(grid, "ACTIVE",   s.activeCells);
    writePointSet(grid, "SELECTED", s.selectedCells);

    // --- SAVES (A–Z chord slots etc.) ---
    juce::ValueTree savesVT("SAVES");

    for (int i = 0; i < (int) s.saves.size(); ++i)
    {
        juce::ValueTree slot("S");
        slot.setProperty("i", i, nullptr);

        const char mode = s.saves[(size_t)i].mode; // 0, 's', 'a'
        slot.setProperty("mode",
                         mode != 0 ? juce::String::charToString(mode) : juce::String(),
                         nullptr);

        juce::ValueTree cells("CELLS");
        for (auto p : s.saves[(size_t)i].screenCells)
        {
            juce::ValueTree c("C");
            c.setProperty("x", p.x, nullptr);
            c.setProperty("y", p.y, nullptr);
            cells.addChild(c, -1, nullptr);
        }

        slot.addChild(cells, -1, nullptr);
        savesVT.addChild(slot, -1, nullptr);
    }

    grid.addChild(savesVT, -1, nullptr);
    // --- end SAVES ---

    root.addChild(grid, -1, nullptr);

    std::unique_ptr<juce::XmlElement> xml(root.createXml());
    copyXmlToBinary(*xml, destData);
}

void UnTETeredAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (!xml) return;

    const juce::ValueTree root = juce::ValueTree::fromXml(*xml);
    const juce::ValueTree grid = root.getChildWithName("GRID");
    if (!grid.isValid()) return;

    GridState s;

    s.originFreqHz = (double) grid.getProperty("originHz", 220.0);
    s.offsetX      = (int)    grid.getProperty("offsetX",  0);
    s.offsetY      = (int)    grid.getProperty("offsetY",  0);

    auto readPointSet = [](juce::ValueTree node)
    {
        PointSet out;
        if (!node.isValid()) return out;

        for (int i = 0; i < node.getNumChildren(); ++i)
        {
            const auto c = node.getChild(i);
            out.insert({ (int)c.getProperty("x", 0),
                         (int)c.getProperty("y", 0) });
        }
        return out;
    };

    s.activeCells   = readPointSet(grid.getChildWithName("ACTIVE"));
    s.selectedCells = readPointSet(grid.getChildWithName("SELECTED"));

    // --- SAVES ---
    const auto savesVT = grid.getChildWithName("SAVES");
    if (savesVT.isValid())
    {
        for (int si = 0; si < savesVT.getNumChildren(); ++si)
        {
            const auto slot = savesVT.getChild(si);
            if (!slot.hasType("S")) continue;

            const int idx = (int) slot.getProperty("i", -1);
            if (idx < 0 || idx >= (int)s.saves.size()) continue;

            const auto modeStr = slot.getProperty("mode", "").toString();
            s.saves[(size_t)idx].mode = modeStr.isNotEmpty() ? modeStr[0] : 0;

            s.saves[(size_t)idx].screenCells.clear();
            const auto cells = slot.getChildWithName("CELLS");
            if (cells.isValid())
            {
                for (int j = 0; j < cells.getNumChildren(); ++j)
                {
                    const auto c = cells.getChild(j);
                    s.saves[(size_t)idx].screenCells.push_back(
                        { (int)c.getProperty("x", 0),
                          (int)c.getProperty("y", 0) }
                    );
                }
            }
        }
    }
    // --- end SAVES ---

    setGridState(s);
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

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UnTETeredAudioProcessor();
}
