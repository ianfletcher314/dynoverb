#include "PluginProcessor.h"
#include "PluginEditor.h"

DynoverbAudioProcessor::DynoverbAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Cache parameter pointers
    reverbTypeParam = apvts.getRawParameterValue("reverbType");
    algoModeParam = apvts.getRawParameterValue("algoMode");
    shimmerPitchParam = apvts.getRawParameterValue("shimmerPitch");
    shimmerAmountParam = apvts.getRawParameterValue("shimmerAmount");
    shimmerInfiniteParam = apvts.getRawParameterValue("shimmerInfinite");
    springTensionParam = apvts.getRawParameterValue("springTension");
    springDripParam = apvts.getRawParameterValue("springDrip");
    springMixParam = apvts.getRawParameterValue("springMix");
    gateThresholdParam = apvts.getRawParameterValue("gateThreshold");
    gateHoldParam = apvts.getRawParameterValue("gateHold");
    gateReleaseParam = apvts.getRawParameterValue("gateRelease");
    gateShapeParam = apvts.getRawParameterValue("gateShape");

    preDelayParam = apvts.getRawParameterValue("preDelay");
    preDelayTempoSyncParam = apvts.getRawParameterValue("preDelayTempoSync");
    preDelaySyncDivParam = apvts.getRawParameterValue("preDelaySyncDiv");
    decayParam = apvts.getRawParameterValue("decay");
    dampingParam = apvts.getRawParameterValue("damping");
    sizeParam = apvts.getRawParameterValue("size");
    diffusionParam = apvts.getRawParameterValue("diffusion");
    modRateParam = apvts.getRawParameterValue("modRate");
    modDepthParam = apvts.getRawParameterValue("modDepth");
    earlyLevelParam = apvts.getRawParameterValue("earlyLevel");
    widthParam = apvts.getRawParameterValue("width");
    highPassParam = apvts.getRawParameterValue("highPass");
    lowPassParam = apvts.getRawParameterValue("lowPass");
    duckingParam = apvts.getRawParameterValue("ducking");
    mixParam = apvts.getRawParameterValue("mix");
    freezeParam = apvts.getRawParameterValue("freeze");
    bypassParam = apvts.getRawParameterValue("bypass");
}

DynoverbAudioProcessor::~DynoverbAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout DynoverbAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Reverb type selection
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("reverbType", 1), "Reverb Type",
        juce::StringArray{ "Algorithmic", "Shimmer", "Spring", "Gated" }, 0));

    // Algorithmic mode
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("algoMode", 1), "Algorithm Mode",
        juce::StringArray{ "Room", "Hall", "Plate", "Chamber" }, 1));

    // Shimmer parameters
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("shimmerPitch", 1), "Shimmer Pitch",
        juce::StringArray{ "Octave Up", "Fifth Up", "Octave Down", "Fifth Down", "Mixed" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("shimmerAmount", 1), "Shimmer Amount",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("shimmerInfinite", 1), "Shimmer Infinite", false));

    // Spring parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("springTension", 1), "Spring Tension",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("springDrip", 1), "Spring Drip",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 30.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("springMix", 1), "Spring Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 70.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Gated parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gateThreshold", 1), "Gate Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -30.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gateHold", 1), "Gate Hold",
        juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f, 0.5f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gateRelease", 1), "Gate Release",
        juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f, 0.5f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gateShape", 1), "Gate Shape",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Global parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("preDelay", 1), "Pre-Delay",
        juce::NormalisableRange<float>(0.0f, 500.0f, 0.1f, 0.5f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("preDelayTempoSync", 1), "Pre-Delay Sync", false));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("preDelaySyncDiv", 1), "Sync Division",
        juce::StringArray{ "1/32", "1/16T", "1/16", "1/8T", "1/8", "1/4T", "1/4", "1/2", "1/1" }, 4));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("decay", 1), "Decay",
        juce::NormalisableRange<float>(0.1f, 30.0f, 0.01f, 0.4f), 2.0f,
        juce::AudioParameterFloatAttributes().withLabel("s")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("damping", 1), "Damping",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("size", 1), "Size",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("diffusion", 1), "Diffusion",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 70.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("modRate", 1), "Mod Rate",
        juce::NormalisableRange<float>(0.01f, 5.0f, 0.01f, 0.5f), 0.5f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("modDepth", 1), "Mod Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 30.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("earlyLevel", 1), "Early Reflections",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("width", 1), "Width",
        juce::NormalisableRange<float>(0.0f, 200.0f, 0.1f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("highPass", 1), "High Pass",
        juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f, 0.4f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lowPass", 1), "Low Pass",
        juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.4f), 12000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("ducking", 1), "Ducking",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("mix", 1), "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 30.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("freeze", 1), "Freeze", false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("bypass", 1), "Bypass", false));

    return { params.begin(), params.end() };
}

const juce::String DynoverbAudioProcessor::getName() const { return JucePlugin_Name; }
bool DynoverbAudioProcessor::acceptsMidi() const { return false; }
bool DynoverbAudioProcessor::producesMidi() const { return false; }
bool DynoverbAudioProcessor::isMidiEffect() const { return false; }
double DynoverbAudioProcessor::getTailLengthSeconds() const { return 10.0; }
int DynoverbAudioProcessor::getNumPrograms() { return 1; }
int DynoverbAudioProcessor::getCurrentProgram() { return 0; }
void DynoverbAudioProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }
const juce::String DynoverbAudioProcessor::getProgramName(int index) { juce::ignoreUnused(index); return {}; }
void DynoverbAudioProcessor::changeProgramName(int index, const juce::String& newName) { juce::ignoreUnused(index, newName); }
bool DynoverbAudioProcessor::hasEditor() const { return true; }

void DynoverbAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    algorithmicReverb.prepare(sampleRate, samplesPerBlock);
    shimmerReverb.prepare(sampleRate, samplesPerBlock);
    springReverb.prepare(sampleRate, samplesPerBlock);
    gatedReverb.prepare(sampleRate, samplesPerBlock);

    crossfadeBuffer.setSize(2, samplesPerBlock);

    // Setup ducking envelope followers
    duckingEnvelopeL.setAttack(sampleRate, 5.0f);
    duckingEnvelopeL.setRelease(sampleRate, 100.0f);
    duckingEnvelopeR.setAttack(sampleRate, 5.0f);
    duckingEnvelopeR.setRelease(sampleRate, 100.0f);
}

void DynoverbAudioProcessor::releaseResources()
{
}

bool DynoverbAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

ReverbType DynoverbAudioProcessor::getCurrentReverbType() const
{
    return static_cast<ReverbType>(static_cast<int>(reverbTypeParam->load()));
}

void DynoverbAudioProcessor::updateReverbParameters()
{
    // Get global parameters
    float preDelay = preDelayParam->load();

    // Handle tempo sync for pre-delay
    if (preDelayTempoSyncParam->load() > 0.5f)
    {
        int syncDiv = static_cast<int>(preDelaySyncDivParam->load());
        // Sync divisions: 1/32, 1/16T, 1/16, 1/8T, 1/8, 1/4T, 1/4, 1/2, 1/1
        const float divisions[9] = { 0.125f, 0.167f, 0.25f, 0.333f, 0.5f, 0.667f, 1.0f, 2.0f, 4.0f };
        float beatsPerMs = 60000.0f / static_cast<float>(currentBPM);
        preDelay = beatsPerMs * divisions[syncDiv];
        preDelay = std::min(preDelay, 500.0f);
    }

    float decay = decayParam->load();
    float dampingVal = dampingParam->load() / 100.0f;
    float sizeVal = sizeParam->load() / 100.0f;
    float diffusionVal = diffusionParam->load() / 100.0f;
    float modRateVal = modRateParam->load();
    float modDepthVal = modDepthParam->load() / 100.0f;
    float earlyLevelVal = earlyLevelParam->load() / 100.0f;
    float widthVal = widthParam->load() / 100.0f;
    float highPassVal = highPassParam->load();
    float lowPassVal = lowPassParam->load();
    float mixVal = mixParam->load() / 100.0f;
    bool frozen = freezeParam->load() > 0.5f;

    // Update algorithmic reverb
    algorithmicReverb.setMode(static_cast<AlgorithmicMode>(static_cast<int>(algoModeParam->load())));
    algorithmicReverb.setPreDelay(preDelay);
    algorithmicReverb.setDecay(decay);
    algorithmicReverb.setDamping(dampingVal);
    algorithmicReverb.setSize(sizeVal);
    algorithmicReverb.setDiffusion(diffusionVal);
    algorithmicReverb.setModRate(modRateVal);
    algorithmicReverb.setModDepth(modDepthVal);
    algorithmicReverb.setEarlyLevel(earlyLevelVal);
    algorithmicReverb.setWidth(widthVal);
    algorithmicReverb.setHighPassFreq(highPassVal);
    algorithmicReverb.setLowPassFreq(lowPassVal);
    algorithmicReverb.setMix(mixVal);
    algorithmicReverb.setFreeze(frozen);

    // Update shimmer reverb
    shimmerReverb.setPitchMode(static_cast<ShimmerPitch>(static_cast<int>(shimmerPitchParam->load())));
    shimmerReverb.setShimmerAmount(shimmerAmountParam->load() / 100.0f);
    shimmerReverb.setInfinite(shimmerInfiniteParam->load() > 0.5f);
    shimmerReverb.setPreDelay(preDelay);
    shimmerReverb.setDecay(decay);
    shimmerReverb.setDamping(dampingVal);
    shimmerReverb.setSize(sizeVal);
    shimmerReverb.setDiffusion(diffusionVal);
    shimmerReverb.setModRate(modRateVal);
    shimmerReverb.setModDepth(modDepthVal);
    shimmerReverb.setWidth(widthVal);
    shimmerReverb.setHighPassFreq(highPassVal);
    shimmerReverb.setLowPassFreq(lowPassVal);
    shimmerReverb.setMix(mixVal);
    shimmerReverb.setFreeze(frozen);

    // Update spring reverb
    springReverb.setTension(springTensionParam->load() / 100.0f);
    springReverb.setDrip(springDripParam->load() / 100.0f);
    springReverb.setSpringMix(springMixParam->load() / 100.0f);
    springReverb.setPreDelay(preDelay);
    springReverb.setDecay(decay);
    springReverb.setDamping(dampingVal);
    springReverb.setSize(sizeVal);
    springReverb.setDiffusion(diffusionVal);
    springReverb.setWidth(widthVal);
    springReverb.setHighPassFreq(highPassVal);
    springReverb.setLowPassFreq(lowPassVal);
    springReverb.setMix(mixVal);
    springReverb.setFreeze(frozen);

    // Update gated reverb
    gatedReverb.setThreshold(gateThresholdParam->load());
    gatedReverb.setHoldTime(gateHoldParam->load());
    gatedReverb.setReleaseTime(gateReleaseParam->load());
    gatedReverb.setGateShape(gateShapeParam->load() / 100.0f);
    gatedReverb.setPreDelay(preDelay);
    gatedReverb.setDecay(decay);
    gatedReverb.setDamping(dampingVal);
    gatedReverb.setSize(sizeVal);
    gatedReverb.setDiffusion(diffusionVal);
    gatedReverb.setEarlyLevel(earlyLevelVal);
    gatedReverb.setWidth(widthVal);
    gatedReverb.setHighPassFreq(highPassVal);
    gatedReverb.setLowPassFreq(lowPassVal);
    gatedReverb.setMix(mixVal);
    gatedReverb.setFreeze(frozen);
}

void DynoverbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Check bypass
    if (bypassParam->load() > 0.5f)
        return;

    // Get playhead info for tempo sync
    if (auto* playhead = getPlayHead())
    {
        if (auto posInfo = playhead->getPosition())
        {
            if (posInfo->getBpm().hasValue())
                currentBPM = *posInfo->getBpm();
        }
    }

    // Measure input level
    float inLevel = 0.0f;
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        inLevel = std::max(inLevel, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    inputLevel.store(inLevel);

    // Update parameters
    updateReverbParameters();

    // Check for type change and setup crossfade
    ReverbType newType = static_cast<ReverbType>(static_cast<int>(reverbTypeParam->load()));
    if (newType != targetType)
    {
        currentType = targetType;
        targetType = newType;
        crossfadePosition = 0.0f;
    }

    // Get ducking amount
    float duckingAmount = duckingParam->load() / 100.0f;

    // Store dry signal for ducking
    juce::AudioBuffer<float> dryBuffer;
    if (duckingAmount > 0.0f)
    {
        dryBuffer.makeCopyOf(buffer);
    }

    // Process based on type (with crossfading if transitioning)
    if (crossfadePosition < 1.0f)
    {
        // Make copy for crossfade
        crossfadeBuffer.makeCopyOf(buffer);

        // Process current type
        switch (currentType)
        {
            case ReverbType::Algorithmic: algorithmicReverb.process(buffer); break;
            case ReverbType::Shimmer: shimmerReverb.process(buffer); break;
            case ReverbType::Spring: springReverb.process(buffer); break;
            case ReverbType::Gated: gatedReverb.process(buffer); break;
        }

        // Process target type
        switch (targetType)
        {
            case ReverbType::Algorithmic: algorithmicReverb.process(crossfadeBuffer); break;
            case ReverbType::Shimmer: shimmerReverb.process(crossfadeBuffer); break;
            case ReverbType::Spring: springReverb.process(crossfadeBuffer); break;
            case ReverbType::Gated: gatedReverb.process(crossfadeBuffer); break;
        }

        // Crossfade sample-by-sample
        int numSamples = buffer.getNumSamples();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* out = buffer.getWritePointer(ch);
            auto* target = crossfadeBuffer.getReadPointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                float fade = std::min(1.0f, crossfadePosition + i * crossfadeRate);
                out[i] = out[i] * (1.0f - fade) + target[i] * fade;
            }
        }

        crossfadePosition += numSamples * crossfadeRate;
        if (crossfadePosition >= 1.0f)
        {
            crossfadePosition = 1.0f;
            currentType = targetType;
        }
    }
    else
    {
        // Process single type
        switch (targetType)
        {
            case ReverbType::Algorithmic: algorithmicReverb.process(buffer); break;
            case ReverbType::Shimmer: shimmerReverb.process(buffer); break;
            case ReverbType::Spring: springReverb.process(buffer); break;
            case ReverbType::Gated: gatedReverb.process(buffer); break;
        }
    }

    // Apply ducking
    if (duckingAmount > 0.0f)
    {
        int numSamples = buffer.getNumSamples();
        auto* leftIn = dryBuffer.getReadPointer(0);
        auto* rightIn = dryBuffer.getNumChannels() > 1 ? dryBuffer.getReadPointer(1) : leftIn;
        auto* leftOut = buffer.getWritePointer(0);
        auto* rightOut = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            float envL = duckingEnvelopeL.process(leftIn[i]);
            float envR = duckingEnvelopeR.process(rightIn[i]);
            float env = std::max(envL, envR);

            // Duck the reverb based on input level
            float duckGain = 1.0f - env * duckingAmount * 3.0f;
            duckGain = std::max(0.0f, duckGain);

            leftOut[i] *= duckGain;
            if (rightOut)
                rightOut[i] *= duckGain;
        }
    }

    // Measure output level
    float outLevel = 0.0f;
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        outLevel = std::max(outLevel, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    outputLevel.store(outLevel);
}

juce::AudioProcessorEditor* DynoverbAudioProcessor::createEditor()
{
    return new DynoverbAudioProcessorEditor(*this);
}

void DynoverbAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DynoverbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DynoverbAudioProcessor();
}
