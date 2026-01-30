#include "GatedReverb.h"
#include <cmath>

GatedReverb::GatedReverb()
{
    // Initialize diffusers
    for (int i = 0; i < numDiffusers; ++i)
    {
        diffusersL[i] = DSPUtils::AllpassFilter(4096);
        diffusersR[i] = DSPUtils::AllpassFilter(4096);
    }
}

void GatedReverb::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    // Allocate pre-delay buffer
    int maxPreDelaySamples = static_cast<int>(sampleRate * 0.5);
    preDelayBufferL.resize(maxPreDelaySamples, 0.0f);
    preDelayBufferR.resize(maxPreDelaySamples, 0.0f);

    // Allocate early reflections buffer
    int maxEarlySamples = static_cast<int>(sampleRate * 0.15);
    earlyBufferL.resize(maxEarlySamples, 0.0f);
    earlyBufferR.resize(maxEarlySamples, 0.0f);

    // Allocate FDN delay lines
    int maxFdnSamples = static_cast<int>(sampleRate * 0.1);
    const int fdnDelays[6] = { 701, 887, 1013, 1153, 1301, 1451 };
    for (int i = 0; i < fdnSize; ++i)
    {
        fdnDelayLines[i].resize(maxFdnSamples, 0.0f);
        fdnDelayLengths[i] = static_cast<int>(fdnDelays[i] * sampleRate / 44100.0);
        fdnDelayLengths[i] = std::clamp(fdnDelayLengths[i], 1, maxFdnSamples - 1);
        fdnWriteIndices[i] = 0;
        fdnFilterStates[i] = 0.0f;
    }

    // Setup envelope followers
    inputEnvelopeL.setAttack(sampleRate, 1.0f);   // Fast attack
    inputEnvelopeL.setRelease(sampleRate, 50.0f);
    inputEnvelopeR.setAttack(sampleRate, 1.0f);
    inputEnvelopeR.setRelease(sampleRate, 50.0f);

    // Setup filters
    auto hpCoeffs = DSPUtils::calcHighPass(sampleRate, highPassFreq);
    auto lpCoeffs = DSPUtils::calcLowPass(sampleRate, lowPassFreq);
    highPassL.setCoefficients(hpCoeffs);
    highPassR.setCoefficients(hpCoeffs);
    lowPassL.setCoefficients(lpCoeffs);
    lowPassR.setCoefficients(lpCoeffs);

    // Mid-frequency boost for 80s character (around 2-3kHz)
    // Simple implementation using peak filter coefficients
    float w0 = 2.0f * 3.14159265358979323846f * 2500.0f / static_cast<float>(sampleRate);
    float cosw0 = std::cos(w0);
    float sinw0 = std::sin(w0);
    float A = std::pow(10.0f, 3.0f / 40.0f);  // 3dB boost
    float alpha = sinw0 / (2.0f * 1.0f);  // Q = 1

    DSPUtils::BiquadCoeffs midCoeffs;
    float a0 = 1.0f + alpha / A;
    midCoeffs.b0 = (1.0f + alpha * A) / a0;
    midCoeffs.b1 = (-2.0f * cosw0) / a0;
    midCoeffs.b2 = (1.0f - alpha * A) / a0;
    midCoeffs.a1 = (-2.0f * cosw0) / a0;
    midCoeffs.a2 = (1.0f - alpha / A) / a0;

    midBoostL.setCoefficients(midCoeffs);
    midBoostR.setCoefficients(midCoeffs);

    updateParameters();
    reset();
}

void GatedReverb::reset()
{
    std::fill(preDelayBufferL.begin(), preDelayBufferL.end(), 0.0f);
    std::fill(preDelayBufferR.begin(), preDelayBufferR.end(), 0.0f);
    preDelayWriteIndex = 0;

    std::fill(earlyBufferL.begin(), earlyBufferL.end(), 0.0f);
    std::fill(earlyBufferR.begin(), earlyBufferR.end(), 0.0f);
    earlyWriteIndex = 0;

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

    highPassL.reset();
    highPassR.reset();
    lowPassL.reset();
    lowPassR.reset();
    midBoostL.reset();
    midBoostR.reset();

    inputEnvelopeL.reset();
    inputEnvelopeR.reset();

    gateEnvelope = 0.0f;
    holdCounter = 0;
    gateOpen = false;
    currentGateLevel.store(0.0f);
}

void GatedReverb::setThreshold(float thresholdDb)
{
    threshold = std::clamp(thresholdDb, -60.0f, 0.0f);
}

void GatedReverb::setHoldTime(float holdMs)
{
    holdTimeMs = std::clamp(holdMs, 10.0f, 500.0f);
    updateParameters();
}

void GatedReverb::setReleaseTime(float releaseMs)
{
    releaseTimeMs = std::clamp(releaseMs, 10.0f, 500.0f);
    updateParameters();
}

void GatedReverb::setGateShape(float shape)
{
    gateShape = std::clamp(shape, 0.0f, 1.0f);
}

void GatedReverb::updateParameters()
{
    // Calculate pre-delay
    preDelaySamples = static_cast<int>(preDelayMs * currentSampleRate / 1000.0);
    preDelaySamples = std::clamp(preDelaySamples, 0, static_cast<int>(preDelayBufferL.size()) - 1);

    // Gate timing
    holdSamples = holdTimeMs * currentSampleRate / 1000.0f;
    releaseCoeff = DSPUtils::calculateCoefficient(currentSampleRate, releaseTimeMs);

    // Setup early reflections (dense, even spacing for 80s character)
    int earlyBufSize = static_cast<int>(earlyBufferL.size()) - 1;
    for (int i = 0; i < numEarlyTaps; ++i)
    {
        // Dense, evenly-spaced reflections
        float delayMs = 5.0f + i * 8.0f * size;  // 5-100ms range typically
        earlyDelaysL[i] = static_cast<int>(delayMs * currentSampleRate / 1000.0f);
        earlyDelaysR[i] = static_cast<int>(delayMs * 1.08f * currentSampleRate / 1000.0f);

        earlyDelaysL[i] = std::clamp(earlyDelaysL[i], 1, earlyBufSize);
        earlyDelaysR[i] = std::clamp(earlyDelaysR[i], 1, earlyBufSize);

        // Flat gain curve (no decay) for punchy character
        earlyGainsL[i] = 0.8f / std::sqrt(static_cast<float>(numEarlyTaps));
        earlyGainsR[i] = 0.8f / std::sqrt(static_cast<float>(numEarlyTaps));

        // Alternate polarity for density
        if (i % 2 == 1)
        {
            earlyGainsL[i] *= -1.0f;
        }
        if ((i + 1) % 2 == 1)
        {
            earlyGainsR[i] *= -1.0f;
        }
    }

    // Update FDN delays based on size
    int maxFdnSamples = static_cast<int>(fdnDelayLines[0].size()) - 1;
    const int fdnDelays[6] = { 701, 887, 1013, 1153, 1301, 1451 };
    for (int i = 0; i < fdnSize; ++i)
    {
        fdnDelayLengths[i] = static_cast<int>(fdnDelays[i] * size * currentSampleRate / 44100.0);
        fdnDelayLengths[i] = std::clamp(fdnDelayLengths[i], 1, maxFdnSamples);
    }

    // Update diffusers
    const int diffuserDelays[4] = { 107, 251, 379, 503 };
    for (int i = 0; i < numDiffusers; ++i)
    {
        int delay = static_cast<int>(diffuserDelays[i] * size * currentSampleRate / 44100.0);
        diffusersL[i].setDelay(std::clamp(delay, 1, 4095));
        diffusersR[i].setDelay(std::clamp(static_cast<int>(delay * 1.1f), 1, 4095));
        diffusersL[i].setFeedback(0.5f + diffusion * 0.3f);
        diffusersR[i].setFeedback(0.5f + diffusion * 0.3f);
    }

    // Update filters
    auto hpCoeffs = DSPUtils::calcHighPass(currentSampleRate, highPassFreq);
    auto lpCoeffs = DSPUtils::calcLowPass(currentSampleRate, lowPassFreq);
    highPassL.setCoefficients(hpCoeffs);
    highPassR.setCoefficients(hpCoeffs);
    lowPassL.setCoefficients(lpCoeffs);
    lowPassR.setCoefficients(lpCoeffs);
}

void GatedReverb::processReverb(float inputL, float inputR, float& outL, float& outR)
{
    // Write to early reflection buffer
    earlyBufferL[earlyWriteIndex] = inputL;
    earlyBufferR[earlyWriteIndex] = inputR;

    // Sum early reflections
    float earlyL = 0.0f;
    float earlyR = 0.0f;
    int earlyBufSize = static_cast<int>(earlyBufferL.size());

    for (int i = 0; i < numEarlyTaps; ++i)
    {
        int readL = earlyWriteIndex - earlyDelaysL[i];
        if (readL < 0) readL += earlyBufSize;
        int readR = earlyWriteIndex - earlyDelaysR[i];
        if (readR < 0) readR += earlyBufSize;

        earlyL += earlyBufferL[readL] * earlyGainsL[i];
        earlyR += earlyBufferR[readR] * earlyGainsR[i];
    }

    earlyWriteIndex++;
    if (earlyWriteIndex >= earlyBufSize) earlyWriteIndex = 0;

    // FDN for dense tail
    std::array<float, fdnSize> delayOutputs;
    for (int i = 0; i < fdnSize; ++i)
    {
        int bufSize = static_cast<int>(fdnDelayLines[i].size());
        int readIndex = fdnWriteIndices[i] - fdnDelayLengths[i];
        if (readIndex < 0) readIndex += bufSize;
        delayOutputs[i] = fdnDelayLines[i][readIndex];
    }

    // Householder mixing
    std::array<float, fdnSize> mixedOutputs;
    for (int i = 0; i < fdnSize; ++i)
    {
        mixedOutputs[i] = 0.0f;
        for (int j = 0; j < fdnSize; ++j)
        {
            mixedOutputs[i] += householder[i][j] * delayOutputs[j];
        }
    }

    // Calculate feedback (relatively high for dense tail)
    float feedback = 0.85f + decaySeconds * 0.05f;
    feedback = std::clamp(feedback, 0.0f, 0.95f);

    // Write back with damping
    for (int i = 0; i < fdnSize; ++i)
    {
        float dampedOutput = mixedOutputs[i] * (1.0f - damping * 0.5f) + fdnFilterStates[i] * damping * 0.5f;
        fdnFilterStates[i] = dampedOutput;

        float feedbackSample = dampedOutput * feedback;
        // Inject early reflections into FDN
        float newSample = (i < 3) ?
            earlyL * 0.3f + feedbackSample :
            earlyR * 0.3f + feedbackSample;

        int bufSize = static_cast<int>(fdnDelayLines[i].size());
        fdnDelayLines[i][fdnWriteIndices[i]] = newSample;

        fdnWriteIndices[i]++;
        if (fdnWriteIndices[i] >= bufSize) fdnWriteIndices[i] = 0;
    }

    // Sum outputs
    float lateL = (mixedOutputs[0] + mixedOutputs[2] + mixedOutputs[4]) * 0.4f;
    float lateR = (mixedOutputs[1] + mixedOutputs[3] + mixedOutputs[5]) * 0.4f;

    // Combine early and late (more early for gated character)
    outL = earlyL * earlyLevel + lateL * (1.0f - earlyLevel * 0.5f);
    outR = earlyR * earlyLevel + lateR * (1.0f - earlyLevel * 0.5f);
}

void GatedReverb::process(juce::AudioBuffer<float>& buffer)
{
    if (bypassed)
        return;

    updateParameters();

    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    int numSamples = buffer.getNumSamples();
    int preDelayBufSize = static_cast<int>(preDelayBufferL.size());

    float thresholdLinear = DSPUtils::decibelsToLinear(threshold);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float inputL = leftChannel[sample];
        float inputR = rightChannel ? rightChannel[sample] : inputL;

        // Envelope follower on input (for gate triggering)
        float envL = inputEnvelopeL.process(inputL);
        float envR = inputEnvelopeR.process(inputR);
        float inputEnvelope = std::max(envL, envR);

        // Gate logic
        if (inputEnvelope > thresholdLinear)
        {
            gateOpen = true;
            holdCounter = static_cast<int>(holdSamples);
            gateEnvelope = 1.0f;
        }
        else if (holdCounter > 0)
        {
            holdCounter--;
        }
        else if (gateOpen)
        {
            // Release phase
            gateEnvelope -= releaseCoeff;

            // Apply shape (0 = linear, 1 = exponential)
            float shapedEnvelope = gateEnvelope;
            if (gateShape > 0.0f)
            {
                // Interpolate between linear and exponential
                float expEnv = std::pow(gateEnvelope, 1.0f + gateShape * 3.0f);
                shapedEnvelope = gateEnvelope * (1.0f - gateShape) + expEnv * gateShape;
            }

            gateEnvelope = shapedEnvelope;

            if (gateEnvelope <= 0.001f)
            {
                gateEnvelope = 0.0f;
                gateOpen = false;
            }
        }

        currentGateLevel.store(gateEnvelope);

        // Input filtering
        float filteredL = highPassL.process(inputL);
        float filteredR = highPassR.process(inputR);

        // Pre-delay
        preDelayBufferL[preDelayWriteIndex] = filteredL;
        preDelayBufferR[preDelayWriteIndex] = filteredR;

        int preDelayReadIndex = preDelayWriteIndex - preDelaySamples;
        if (preDelayReadIndex < 0) preDelayReadIndex += preDelayBufSize;

        float delayedL = preDelayBufferL[preDelayReadIndex];
        float delayedR = preDelayBufferR[preDelayReadIndex];

        preDelayWriteIndex++;
        if (preDelayWriteIndex >= preDelayBufSize) preDelayWriteIndex = 0;

        // Diffusion
        for (int i = 0; i < numDiffusers; ++i)
        {
            delayedL = diffusersL[i].process(delayedL);
            delayedR = diffusersR[i].process(delayedR);
        }

        // Process reverb
        float reverbL, reverbR;
        processReverb(delayedL, delayedR, reverbL, reverbR);

        // Apply gate envelope
        reverbL *= gateEnvelope;
        reverbR *= gateEnvelope;

        // Mid boost for 80s character
        reverbL = midBoostL.process(reverbL);
        reverbR = midBoostR.process(reverbR);

        // Output filtering
        reverbL = lowPassL.process(reverbL);
        reverbR = lowPassR.process(reverbR);

        // Apply width
        float mid = (reverbL + reverbR) * 0.5f;
        float side = (reverbL - reverbR) * 0.5f;
        float wetL = mid + side * width;
        float wetR = mid - side * width;

        // Mix
        float dryL = leftChannel[sample];
        float dryR = rightChannel ? rightChannel[sample] : dryL;

        leftChannel[sample] = dryL * (1.0f - mix) + wetL * mix;
        if (rightChannel)
            rightChannel[sample] = dryR * (1.0f - mix) + wetR * mix;
    }
}
