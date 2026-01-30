#pragma once

#include <JuceHeader.h>

// Base class for all reverb types
class ReverbBase
{
public:
    virtual ~ReverbBase() = default;

    virtual void prepare(double sampleRate, int samplesPerBlock) = 0;
    virtual void process(juce::AudioBuffer<float>& buffer) = 0;
    virtual void reset() = 0;

    // Common parameters all reverbs share
    void setPreDelay(float preDelayMs) { this->preDelayMs = preDelayMs; }
    void setDecay(float decaySeconds) { this->decaySeconds = decaySeconds; }
    void setDamping(float dampingAmount) { this->damping = dampingAmount; }
    void setSize(float roomSize) { this->size = roomSize; }
    void setDiffusion(float diffusionAmount) { this->diffusion = diffusionAmount; }
    void setModRate(float rate) { this->modRate = rate; }
    void setModDepth(float depth) { this->modDepth = depth; }
    void setEarlyLevel(float level) { this->earlyLevel = level; }
    void setWidth(float stereoWidth) { this->width = stereoWidth; }
    void setHighPassFreq(float freq) { this->highPassFreq = freq; }
    void setLowPassFreq(float freq) { this->lowPassFreq = freq; }
    void setMix(float wetDryMix) { this->mix = wetDryMix; }
    void setFreeze(bool frozen) { this->freeze = frozen; }
    void setBypass(bool shouldBypass) { bypassed = shouldBypass; }

    bool isBypassed() const { return bypassed; }
    bool isFrozen() const { return freeze; }

protected:
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    bool bypassed = false;

    // Common reverb parameters
    float preDelayMs = 0.0f;
    float decaySeconds = 2.0f;
    float damping = 0.5f;
    float size = 0.5f;
    float diffusion = 0.7f;
    float modRate = 0.5f;
    float modDepth = 0.3f;
    float earlyLevel = 0.5f;
    float width = 1.0f;
    float highPassFreq = 20.0f;
    float lowPassFreq = 20000.0f;
    float mix = 0.5f;
    bool freeze = false;
};
