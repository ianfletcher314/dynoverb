#include "ShimmerReverb.h"
#include <cmath>

ShimmerReverb::ShimmerReverb()
{
    // Initialize diffusers
    for (int i = 0; i < numDiffusers; ++i)
    {
        diffusersL[i] = DSPUtils::AllpassFilter(4096);
        diffusersR[i] = DSPUtils::AllpassFilter(4096);
    }

    // Initialize modulated delay lines
    modulatedDelays[0] = DSPUtils::ModulatedDelayLine(48000);
    modulatedDelays[1] = DSPUtils::ModulatedDelayLine(48000);
}

void ShimmerReverb::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    // Allocate grain buffers
    for (int ch = 0; ch < 2; ++ch)
    {
        grainBuffers[ch].resize(grainBufferSize, 0.0f);
        grainWriteIndices[ch] = 0;

        // Initialize grains with staggered positions
        for (int g = 0; g < numGrains; ++g)
        {
            grains[ch][g].readPosition = 0.0f;
            grains[ch][g].readIncrement = 1.0f;
            grains[ch][g].startOffset = g * (grainSize / numGrains);
            grains[ch][g].amplitude = 0.0f;
            grains[ch][g].age = g * (grainSize / numGrains);
        }
    }

    // Create Hann window
    hannWindow.resize(grainSize);
    for (int i = 0; i < grainSize; ++i)
    {
        hannWindow[i] = 0.5f * (1.0f - std::cos(2.0f * 3.14159265358979323846f * i / (grainSize - 1)));
    }

    // Allocate FDN delay lines
    int maxFdnSamples = static_cast<int>(sampleRate * 0.15);
    const int fdnDelays[4] = { 1087, 1423, 1777, 2131 };
    for (int i = 0; i < fdnSize; ++i)
    {
        fdnDelayLines[i].resize(maxFdnSamples, 0.0f);
        fdnDelayLengths[i] = static_cast<int>(fdnDelays[i] * sampleRate / 44100.0);
        fdnDelayLengths[i] = std::clamp(fdnDelayLengths[i], 1, maxFdnSamples - 1);
        fdnWriteIndices[i] = 0;
        fdnFilterStates[i] = 0.0f;
    }

    // Setup filters
    auto hpCoeffs = DSPUtils::calcHighPass(sampleRate, highPassFreq);
    auto lpCoeffs = DSPUtils::calcLowPass(sampleRate, lowPassFreq);
    highPassL.setCoefficients(hpCoeffs);
    highPassR.setCoefficients(hpCoeffs);
    lowPassL.setCoefficients(lpCoeffs);
    lowPassR.setCoefficients(lpCoeffs);

    // Setup modulated delays
    modulatedDelays[0].setDelay(static_cast<float>(sampleRate * 0.03f));
    modulatedDelays[1].setDelay(static_cast<float>(sampleRate * 0.033f));
    modulatedDelays[0].setModDepth(sampleRate * 0.003f);
    modulatedDelays[1].setModDepth(sampleRate * 0.003f);

    updateParameters();
    reset();
}

void ShimmerReverb::reset()
{
    for (int ch = 0; ch < 2; ++ch)
    {
        std::fill(grainBuffers[ch].begin(), grainBuffers[ch].end(), 0.0f);
        grainWriteIndices[ch] = 0;
    }

    for (int i = 0; i < fdnSize; ++i)
    {
        std::fill(fdnDelayLines[i].begin(), fdnDelayLines[i].end(), 0.0f);
        fdnWriteIndices[i] = 0;
        fdnFilterStates[i] = 0.0f;
    }

    for (int i = 0; i < numDiffusers; ++i)
    {
        diffusersL[i].reset();
        diffusersR[i].reset();
    }

    modulatedDelays[0].reset();
    modulatedDelays[1].reset();

    highPassL.reset();
    highPassR.reset();
    lowPassL.reset();
    lowPassR.reset();

    feedbackAccumL = 0.0f;
    feedbackAccumR = 0.0f;
    lfoPhase = 0.0f;
    grainTriggerCounter = 0;
    currentGrain = 0;
}

void ShimmerReverb::setPitchMode(ShimmerPitch newMode)
{
    pitchMode = newMode;
    updateParameters();
}

void ShimmerReverb::setShimmerAmount(float amount)
{
    shimmerAmount = std::clamp(amount, 0.0f, 1.0f);
}

void ShimmerReverb::setInfinite(bool infinite)
{
    infiniteMode = infinite;
}

float ShimmerReverb::getPitchRatio() const
{
    switch (pitchMode)
    {
        case ShimmerPitch::OctaveUp:   return 2.0f;
        case ShimmerPitch::FifthUp:    return 1.5f;
        case ShimmerPitch::OctaveDown: return 0.5f;
        case ShimmerPitch::FifthDown:  return 0.667f;
        case ShimmerPitch::Mixed:      return 2.0f;
        default: return 1.0f;
    }
}

float ShimmerReverb::getSecondaryPitchRatio() const
{
    if (pitchMode == ShimmerPitch::Mixed)
        return 1.5f;  // Fifth up as secondary
    return getPitchRatio();
}

void ShimmerReverb::updateParameters()
{
    // Update diffusers
    const int diffuserDelays[4] = { 113, 337, 509, 701 };
    for (int i = 0; i < numDiffusers; ++i)
    {
        int delay = static_cast<int>(diffuserDelays[i] * size * currentSampleRate / 44100.0);
        diffusersL[i].setDelay(std::clamp(delay, 1, 4095));
        diffusersR[i].setDelay(std::clamp(static_cast<int>(delay * 1.12f), 1, 4095));
        diffusersL[i].setFeedback(0.4f + diffusion * 0.35f);
        diffusersR[i].setFeedback(0.4f + diffusion * 0.35f);
    }

    // Update FDN delay lengths based on size
    const int fdnDelays[4] = { 1087, 1423, 1777, 2131 };
    int maxFdnSamples = static_cast<int>(fdnDelayLines[0].size());
    for (int i = 0; i < fdnSize; ++i)
    {
        fdnDelayLengths[i] = static_cast<int>(fdnDelays[i] * size * currentSampleRate / 44100.0);
        fdnDelayLengths[i] = std::clamp(fdnDelayLengths[i], 1, maxFdnSamples - 1);
    }

    // Update filters
    auto hpCoeffs = DSPUtils::calcHighPass(currentSampleRate, highPassFreq);
    auto lpCoeffs = DSPUtils::calcLowPass(currentSampleRate, lowPassFreq);
    highPassL.setCoefficients(hpCoeffs);
    highPassR.setCoefficients(hpCoeffs);
    lowPassL.setCoefficients(lpCoeffs);
    lowPassR.setCoefficients(lpCoeffs);

    // Update modulated delays
    modulatedDelays[0].setDelay(static_cast<float>(currentSampleRate * 0.025f * size));
    modulatedDelays[1].setDelay(static_cast<float>(currentSampleRate * 0.028f * size));
    modulatedDelays[0].setModDepth(currentSampleRate * 0.002f * modDepth);
    modulatedDelays[1].setModDepth(currentSampleRate * 0.002f * modDepth);
}

void ShimmerReverb::processGranular(float inputL, float inputR, float& outL, float& outR)
{
    float pitchRatio = getPitchRatio();
    float pitchRatio2 = getSecondaryPitchRatio();

    // Write input to grain buffers
    grainBuffers[0][grainWriteIndices[0]] = inputL;
    grainBuffers[1][grainWriteIndices[1]] = inputR;

    outL = 0.0f;
    outR = 0.0f;

    // Process each grain
    for (int ch = 0; ch < 2; ++ch)
    {
        float channelOut = 0.0f;

        for (int g = 0; g < numGrains; ++g)
        {
            Grain& grain = grains[ch][g];

            // Calculate window position and apply envelope
            int windowPos = grain.age % grainSize;
            float envelope = hannWindow[windowPos];

            // Read from buffer with interpolation
            float readPos = grain.readPosition;
            int readIdx1 = static_cast<int>(readPos) % grainBufferSize;
            int readIdx2 = (readIdx1 + 1) % grainBufferSize;
            float frac = readPos - static_cast<float>(static_cast<int>(readPos));

            float sample = grainBuffers[ch][readIdx1] * (1.0f - frac) +
                          grainBuffers[ch][readIdx2] * frac;

            channelOut += sample * envelope * grain.amplitude;

            // Advance grain read position (pitch shift)
            float grainPitch = (g % 2 == 0 || pitchMode != ShimmerPitch::Mixed) ? pitchRatio : pitchRatio2;
            grain.readPosition += grainPitch;
            if (grain.readPosition >= grainBufferSize)
                grain.readPosition -= grainBufferSize;

            grain.age++;

            // Reset grain when it reaches the end
            if (grain.age >= grainSize)
            {
                grain.age = 0;
                grain.readPosition = static_cast<float>(grainWriteIndices[ch] - grainSize / 2);
                if (grain.readPosition < 0) grain.readPosition += grainBufferSize;
                grain.amplitude = 1.0f / numGrains;
            }
        }

        if (ch == 0)
            outL = channelOut;
        else
            outR = channelOut;
    }

    // Advance write indices
    grainWriteIndices[0] = (grainWriteIndices[0] + 1) % grainBufferSize;
    grainWriteIndices[1] = (grainWriteIndices[1] + 1) % grainBufferSize;
}

void ShimmerReverb::process(juce::AudioBuffer<float>& buffer)
{
    if (bypassed)
        return;

    updateParameters();

    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    int numSamples = buffer.getNumSamples();

    // Calculate feedback
    float feedback;
    if (freeze || infiniteMode || decaySeconds >= 30.0f)
    {
        feedback = 0.998f;
    }
    else
    {
        float avgDelay = 0.0f;
        for (int i = 0; i < fdnSize; ++i)
            avgDelay += fdnDelayLengths[i];
        avgDelay /= fdnSize;

        feedback = std::pow(10.0f, -3.0f * avgDelay / (decaySeconds * currentSampleRate));
        feedback = std::clamp(feedback, 0.0f, 0.995f);
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float inputL = leftChannel[sample];
        float inputR = rightChannel ? rightChannel[sample] : inputL;

        // Input filtering
        inputL = highPassL.process(inputL);
        inputR = highPassR.process(inputR);

        // Add feedback from previous iteration
        float feedbackInputL = inputL + feedbackAccumL * feedback;
        float feedbackInputR = inputR + feedbackAccumR * feedback;

        // Diffusion
        for (int i = 0; i < numDiffusers; ++i)
        {
            feedbackInputL = diffusersL[i].process(feedbackInputL);
            feedbackInputR = diffusersR[i].process(feedbackInputR);
        }

        // Pitch shifting (granular)
        float shiftedL, shiftedR;
        processGranular(feedbackInputL, feedbackInputR, shiftedL, shiftedR);

        // Blend original and pitch-shifted
        float blendL = feedbackInputL * (1.0f - shimmerAmount) + shiftedL * shimmerAmount;
        float blendR = feedbackInputR * (1.0f - shimmerAmount) + shiftedR * shimmerAmount;

        // Modulation LFO
        float lfo1 = std::sin(lfoPhase * 6.283185307179586f);
        float lfo2 = std::sin((lfoPhase + 0.25f) * 6.283185307179586f);
        lfoPhase += modRate / static_cast<float>(currentSampleRate);
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        // Modulated delays
        float modDelayL = modulatedDelays[0].process(blendL, lfo1);
        float modDelayR = modulatedDelays[1].process(blendR, lfo2);

        // FDN processing
        std::array<float, fdnSize> delayOutputs;
        for (int i = 0; i < fdnSize; ++i)
        {
            int bufSize = static_cast<int>(fdnDelayLines[i].size());
            int readIndex = fdnWriteIndices[i] - fdnDelayLengths[i];
            if (readIndex < 0) readIndex += bufSize;
            delayOutputs[i] = fdnDelayLines[i][readIndex];
        }

        // Simple Householder mixing
        float sum = 0.0f;
        for (int i = 0; i < fdnSize; ++i)
            sum += delayOutputs[i];
        sum *= 0.5f;

        std::array<float, fdnSize> mixedOutputs;
        for (int i = 0; i < fdnSize; ++i)
        {
            mixedOutputs[i] = sum - delayOutputs[i];
        }

        // Write back to FDN with damping
        for (int i = 0; i < fdnSize; ++i)
        {
            float dampedOutput = mixedOutputs[i] * (1.0f - damping) + fdnFilterStates[i] * damping;
            fdnFilterStates[i] = dampedOutput;

            float feedbackSample = dampedOutput * feedback;
            float newSample = (i < 2) ? modDelayL * 0.5f + feedbackSample : modDelayR * 0.5f + feedbackSample;

            int bufSize = static_cast<int>(fdnDelayLines[i].size());
            fdnDelayLines[i][fdnWriteIndices[i]] = newSample;

            fdnWriteIndices[i]++;
            if (fdnWriteIndices[i] >= bufSize) fdnWriteIndices[i] = 0;
        }

        // Sum FDN outputs for stereo
        float fdnOutL = (mixedOutputs[0] + mixedOutputs[2]) * 0.5f;
        float fdnOutR = (mixedOutputs[1] + mixedOutputs[3]) * 0.5f;

        // Store for next feedback iteration
        feedbackAccumL = fdnOutL;
        feedbackAccumR = fdnOutR;

        // Apply width
        float mid = (fdnOutL + fdnOutR) * 0.5f;
        float side = (fdnOutL - fdnOutR) * 0.5f;
        float wetL = mid + side * width;
        float wetR = mid - side * width;

        // Output filtering
        wetL = lowPassL.process(wetL);
        wetR = lowPassR.process(wetR);

        // Mix
        float dryL = leftChannel[sample];
        float dryR = rightChannel ? rightChannel[sample] : dryL;

        leftChannel[sample] = dryL * (1.0f - mix) + wetL * mix;
        if (rightChannel)
            rightChannel[sample] = dryR * (1.0f - mix) + wetR * mix;
    }
}
