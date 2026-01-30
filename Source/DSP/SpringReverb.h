#pragma once

#include <JuceHeader.h>
#include "ReverbBase.h"
#include "DSPUtils.h"
#include <array>

class SpringReverb : public ReverbBase
{
public:
    SpringReverb();

    void prepare(double sampleRate, int samplesPerBlock) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    // Spring-specific parameters
    void setTension(float tensionAmount);    // 0-1, affects pitch/chirp
    void setDrip(float dripAmount);          // 0-1, spring "splatter" effect
    void setSpringMix(float springAmount);   // 0-1, blend of spring character

    float getTension() const { return tension; }
    float getDrip() const { return drip; }
    float getSpringMix() const { return springMix; }

private:
    void updateParameters();
    float processSpringModel(float input, int channel);

    // Spring-specific parameters
    float tension = 0.5f;    // Affects dispersive delay
    float drip = 0.3f;       // Splatter/chaos amount
    float springMix = 0.7f;  // How much spring character

    // Physical spring model parameters
    // Springs have dispersive characteristics - high frequencies travel faster
    static constexpr int numSprings = 3;  // Simulate 3 springs for richness

    // Dispersive delay network for each spring
    struct SpringDelay
    {
        std::vector<float> delayLine;
        std::vector<float> allpassBuffer;
        int delayLength = 1000;
        int writeIndex = 0;
        int allpassWriteIndex = 0;
        float filterState = 0.0f;
        float allpassState = 0.0f;

        // Chirp filter states
        float chirpState1 = 0.0f;
        float chirpState2 = 0.0f;
    };

    std::array<SpringDelay, numSprings> springsL;
    std::array<SpringDelay, numSprings> springsR;

    // Delay lengths for each spring (create natural beating)
    const int baseDelayLengths[3] = { 1103, 1327, 1559 };  // Prime-based

    // Pre-delay
    std::vector<float> preDelayBufferL;
    std::vector<float> preDelayBufferR;
    int preDelayWriteIndex = 0;
    int preDelaySamples = 0;

    // Tank diffusers (for smoothing)
    static constexpr int numDiffusers = 3;
    std::array<DSPUtils::AllpassFilter, numDiffusers> diffusersL;
    std::array<DSPUtils::AllpassFilter, numDiffusers> diffusersR;

    // Drip modulation (chaotic LFO)
    float dripPhase = 0.0f;
    float dripNoise = 0.0f;

    // Filters
    DSPUtils::BiquadFilter highPassL, highPassR;
    DSPUtils::BiquadFilter lowPassL, lowPassR;

    // One-pole filters for spring response shaping
    DSPUtils::OnePoleFilter springLPL, springLPR;

    // Random number generator for drip
    juce::Random random;
};
