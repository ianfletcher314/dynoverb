#pragma once

#include <JuceHeader.h>
#include "ReverbBase.h"
#include "DSPUtils.h"
#include <array>

// Algorithmic reverb modes
enum class AlgorithmicMode
{
    Room = 0,
    Hall,
    Plate,
    Chamber
};

class AlgorithmicReverb : public ReverbBase
{
public:
    AlgorithmicReverb();

    void prepare(double sampleRate, int samplesPerBlock) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    void setMode(AlgorithmicMode newMode);
    AlgorithmicMode getMode() const { return mode; }

private:
    void updateParameters();
    void processEarlyReflections(float inputL, float inputR, float& outL, float& outR);
    void processFDN(float inputL, float inputR, float& outL, float& outR);

    AlgorithmicMode mode = AlgorithmicMode::Hall;

    // Pre-delay buffers
    std::vector<float> preDelayBufferL;
    std::vector<float> preDelayBufferR;
    int preDelayWriteIndex = 0;
    int preDelaySamples = 0;

    // Early reflections (8 taps per channel)
    static constexpr int numEarlyTaps = 8;
    std::array<int, numEarlyTaps> earlyDelaysL;
    std::array<int, numEarlyTaps> earlyDelaysR;
    std::array<float, numEarlyTaps> earlyGainsL;
    std::array<float, numEarlyTaps> earlyGainsR;
    std::vector<float> earlyBufferL;
    std::vector<float> earlyBufferR;
    int earlyWriteIndex = 0;

    // Feedback Delay Network (8x8 FDN)
    static constexpr int fdnSize = 8;
    std::array<std::vector<float>, fdnSize> fdnDelayLines;
    std::array<int, fdnSize> fdnDelayLengths;
    std::array<int, fdnSize> fdnWriteIndices;
    std::array<float, fdnSize> fdnFilterStates;

    // Allpass diffusers (4 per channel)
    static constexpr int numDiffusers = 4;
    std::array<DSPUtils::AllpassFilter, numDiffusers> diffusersL;
    std::array<DSPUtils::AllpassFilter, numDiffusers> diffusersR;

    // Modulation LFOs
    float lfoPhase = 0.0f;
    float lfoPhaseIncrement = 0.0f;

    // Input/output filters
    DSPUtils::BiquadFilter highPassL, highPassR;
    DSPUtils::BiquadFilter lowPassL, lowPassR;

    // Hadamard matrix for FDN mixing (8x8 normalized)
    static constexpr float hadamard[8][8] = {
        { 0.3535534f,  0.3535534f,  0.3535534f,  0.3535534f,  0.3535534f,  0.3535534f,  0.3535534f,  0.3535534f},
        { 0.3535534f, -0.3535534f,  0.3535534f, -0.3535534f,  0.3535534f, -0.3535534f,  0.3535534f, -0.3535534f},
        { 0.3535534f,  0.3535534f, -0.3535534f, -0.3535534f,  0.3535534f,  0.3535534f, -0.3535534f, -0.3535534f},
        { 0.3535534f, -0.3535534f, -0.3535534f,  0.3535534f,  0.3535534f, -0.3535534f, -0.3535534f,  0.3535534f},
        { 0.3535534f,  0.3535534f,  0.3535534f,  0.3535534f, -0.3535534f, -0.3535534f, -0.3535534f, -0.3535534f},
        { 0.3535534f, -0.3535534f,  0.3535534f, -0.3535534f, -0.3535534f,  0.3535534f, -0.3535534f,  0.3535534f},
        { 0.3535534f,  0.3535534f, -0.3535534f, -0.3535534f, -0.3535534f, -0.3535534f,  0.3535534f,  0.3535534f},
        { 0.3535534f, -0.3535534f, -0.3535534f,  0.3535534f, -0.3535534f,  0.3535534f,  0.3535534f, -0.3535534f}
    };

    // Mode-dependent delay time bases (in ms)
    struct ModeSettings
    {
        float earlySpacing;
        float fdnBaseDelay;
        float fdnSpread;
        float densityFactor;
    };

    static constexpr ModeSettings modeSettings[4] = {
        { 5.0f,  20.0f, 1.2f, 0.8f },   // Room - tight, small
        { 15.0f, 60.0f, 1.5f, 1.0f },   // Hall - spacious
        { 8.0f,  35.0f, 1.1f, 1.2f },   // Plate - dense, metallic
        { 12.0f, 45.0f, 1.3f, 0.9f }    // Chamber - warm, medium
    };
};
