#pragma once

#include <JuceHeader.h>
#include "ReverbBase.h"
#include "DSPUtils.h"
#include <array>
#include <complex>

// Shimmer pitch shift modes
enum class ShimmerPitch
{
    OctaveUp = 0,
    FifthUp,
    OctaveDown,
    FifthDown,
    Mixed  // Octave + Fifth combined
};

class ShimmerReverb : public ReverbBase
{
public:
    ShimmerReverb();

    void prepare(double sampleRate, int samplesPerBlock) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    void setPitchMode(ShimmerPitch newMode);
    void setShimmerAmount(float amount);  // 0-1 blend of pitched signal
    void setInfinite(bool infinite);

    ShimmerPitch getPitchMode() const { return pitchMode; }
    bool isInfinite() const { return infiniteMode; }

private:
    void updateParameters();
    float pitchShift(float input, float pitchRatio, int channel);
    void processGranular(float inputL, float inputR, float& outL, float& outR);

    ShimmerPitch pitchMode = ShimmerPitch::OctaveUp;
    float shimmerAmount = 0.5f;
    bool infiniteMode = false;

    // Granular pitch shifter buffers
    static constexpr int grainBufferSize = 8192;
    static constexpr int numGrains = 4;
    static constexpr int grainSize = 2048;

    std::array<std::vector<float>, 2> grainBuffers;  // L/R
    std::array<int, 2> grainWriteIndices = { 0, 0 };

    // Grain playback state
    struct Grain
    {
        float readPosition = 0.0f;
        float readIncrement = 1.0f;
        int startOffset = 0;
        float amplitude = 0.0f;
        int age = 0;
    };

    std::array<std::array<Grain, numGrains>, 2> grains;  // [channel][grain]
    int grainTriggerCounter = 0;
    int currentGrain = 0;

    // Reverb network (simplified FDN for shimmer)
    static constexpr int fdnSize = 4;
    std::array<std::vector<float>, fdnSize> fdnDelayLines;
    std::array<int, fdnSize> fdnDelayLengths;
    std::array<int, fdnSize> fdnWriteIndices;
    std::array<float, fdnSize> fdnFilterStates;

    // Modulated delay lines for lush sound
    std::array<DSPUtils::ModulatedDelayLine, 2> modulatedDelays;

    // Diffusers
    static constexpr int numDiffusers = 4;
    std::array<DSPUtils::AllpassFilter, numDiffusers> diffusersL;
    std::array<DSPUtils::AllpassFilter, numDiffusers> diffusersR;

    // Filters
    DSPUtils::BiquadFilter highPassL, highPassR;
    DSPUtils::BiquadFilter lowPassL, lowPassR;

    // LFO for modulation
    float lfoPhase = 0.0f;

    // Feedback accumulator for infinite mode
    float feedbackAccumL = 0.0f;
    float feedbackAccumR = 0.0f;

    // Pitch ratios
    float getPitchRatio() const;
    float getSecondaryPitchRatio() const;

    // Hann window for grain envelope
    std::vector<float> hannWindow;
};
