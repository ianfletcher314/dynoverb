#pragma once

#include <JuceHeader.h>
#include "ReverbBase.h"
#include "DSPUtils.h"
#include <array>

class GatedReverb : public ReverbBase
{
public:
    GatedReverb();

    void prepare(double sampleRate, int samplesPerBlock) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    // Gate-specific parameters
    void setThreshold(float thresholdDb);    // -60 to 0 dB
    void setHoldTime(float holdMs);          // 10-500ms
    void setReleaseTime(float releaseMs);    // 10-500ms
    void setGateShape(float shape);          // 0-1, affects envelope shape

    float getThreshold() const { return threshold; }
    float getHoldTime() const { return holdTimeMs; }
    float getReleaseTime() const { return releaseTimeMs; }
    float getGateShape() const { return gateShape; }

    // Get current gate state for visualization
    float getGateLevel() const { return currentGateLevel.load(); }

private:
    void updateParameters();
    void processReverb(float inputL, float inputR, float& outL, float& outR);

    // Gate parameters
    float threshold = -30.0f;      // dB
    float holdTimeMs = 100.0f;     // ms
    float releaseTimeMs = 100.0f;  // ms
    float gateShape = 0.5f;        // Linear to exponential

    // Gate state
    float gateEnvelope = 0.0f;
    int holdCounter = 0;
    bool gateOpen = false;
    std::atomic<float> currentGateLevel { 0.0f };

    // Envelope follower for input
    DSPUtils::EnvelopeFollower inputEnvelopeL;
    DSPUtils::EnvelopeFollower inputEnvelopeR;

    // Coefficients
    float holdSamples = 0;
    float releaseCoeff = 0.0f;

    // Pre-delay
    std::vector<float> preDelayBufferL;
    std::vector<float> preDelayBufferR;
    int preDelayWriteIndex = 0;
    int preDelaySamples = 0;

    // Early reflections (dense for 80s sound)
    static constexpr int numEarlyTaps = 12;
    std::array<int, numEarlyTaps> earlyDelaysL;
    std::array<int, numEarlyTaps> earlyDelaysR;
    std::array<float, numEarlyTaps> earlyGainsL;
    std::array<float, numEarlyTaps> earlyGainsR;
    std::vector<float> earlyBufferL;
    std::vector<float> earlyBufferR;
    int earlyWriteIndex = 0;

    // Dense reverb tail (cut short by gate)
    static constexpr int fdnSize = 6;
    std::array<std::vector<float>, fdnSize> fdnDelayLines;
    std::array<int, fdnSize> fdnDelayLengths;
    std::array<int, fdnSize> fdnWriteIndices;
    std::array<float, fdnSize> fdnFilterStates;

    // Diffusers
    static constexpr int numDiffusers = 4;
    std::array<DSPUtils::AllpassFilter, numDiffusers> diffusersL;
    std::array<DSPUtils::AllpassFilter, numDiffusers> diffusersR;

    // Filters
    DSPUtils::BiquadFilter highPassL, highPassR;
    DSPUtils::BiquadFilter lowPassL, lowPassR;

    // Additional mid-frequency boost for 80s character
    DSPUtils::BiquadFilter midBoostL, midBoostR;

    // Householder mixing matrix for 6x6 FDN
    static constexpr float householder[6][6] = {
        { 0.6667f, -0.3333f, -0.3333f, -0.3333f, -0.3333f, -0.3333f },
        {-0.3333f,  0.6667f, -0.3333f, -0.3333f, -0.3333f, -0.3333f },
        {-0.3333f, -0.3333f,  0.6667f, -0.3333f, -0.3333f, -0.3333f },
        {-0.3333f, -0.3333f, -0.3333f,  0.6667f, -0.3333f, -0.3333f },
        {-0.3333f, -0.3333f, -0.3333f, -0.3333f,  0.6667f, -0.3333f },
        {-0.3333f, -0.3333f, -0.3333f, -0.3333f, -0.3333f,  0.6667f }
    };
};
