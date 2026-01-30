#pragma once

#include <JuceHeader.h>
#include "DSP/AlgorithmicReverb.h"
#include "DSP/ShimmerReverb.h"
#include "DSP/SpringReverb.h"
#include "DSP/GatedReverb.h"

// Reverb type enumeration
enum class ReverbType
{
    Algorithmic = 0,
    Shimmer,
    Spring,
    Gated
};

class DynoverbAudioProcessor : public juce::AudioProcessor
{
public:
    DynoverbAudioProcessor();
    ~DynoverbAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Public API for editor
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Metering
    float getInputLevel() const { return inputLevel.load(); }
    float getOutputLevel() const { return outputLevel.load(); }
    float getGateLevel() const { return gatedReverb.getGateLevel(); }

    // Current reverb type
    ReverbType getCurrentReverbType() const;

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP modules
    AlgorithmicReverb algorithmicReverb;
    ShimmerReverb shimmerReverb;
    SpringReverb springReverb;
    GatedReverb gatedReverb;

    // Cross-fade state for smooth type switching
    ReverbType currentType = ReverbType::Algorithmic;
    ReverbType targetType = ReverbType::Algorithmic;
    float crossfadePosition = 1.0f;  // 1.0 = fully on target
    static constexpr float crossfadeRate = 0.002f;

    // Buffers for crossfading
    juce::AudioBuffer<float> crossfadeBuffer;

    // Parameter pointers
    std::atomic<float>* reverbTypeParam = nullptr;
    std::atomic<float>* algoModeParam = nullptr;
    std::atomic<float>* shimmerPitchParam = nullptr;
    std::atomic<float>* shimmerAmountParam = nullptr;
    std::atomic<float>* shimmerInfiniteParam = nullptr;
    std::atomic<float>* springTensionParam = nullptr;
    std::atomic<float>* springDripParam = nullptr;
    std::atomic<float>* springMixParam = nullptr;
    std::atomic<float>* gateThresholdParam = nullptr;
    std::atomic<float>* gateHoldParam = nullptr;
    std::atomic<float>* gateReleaseParam = nullptr;
    std::atomic<float>* gateShapeParam = nullptr;

    // Global parameters
    std::atomic<float>* preDelayParam = nullptr;
    std::atomic<float>* preDelayTempoSyncParam = nullptr;
    std::atomic<float>* preDelaySyncDivParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* dampingParam = nullptr;
    std::atomic<float>* sizeParam = nullptr;
    std::atomic<float>* diffusionParam = nullptr;
    std::atomic<float>* modRateParam = nullptr;
    std::atomic<float>* modDepthParam = nullptr;
    std::atomic<float>* earlyLevelParam = nullptr;
    std::atomic<float>* widthParam = nullptr;
    std::atomic<float>* highPassParam = nullptr;
    std::atomic<float>* lowPassParam = nullptr;
    std::atomic<float>* duckingParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* freezeParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;

    // Ducking envelope follower
    DSPUtils::EnvelopeFollower duckingEnvelopeL;
    DSPUtils::EnvelopeFollower duckingEnvelopeR;

    // Metering
    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };

    // Playhead info for tempo sync
    double currentBPM = 120.0;

    // Update reverb parameters from APVTS
    void updateReverbParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynoverbAudioProcessor)
};
