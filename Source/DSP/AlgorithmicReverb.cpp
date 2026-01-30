#include "AlgorithmicReverb.h"

AlgorithmicReverb::AlgorithmicReverb()
{
    // Initialize diffusers with default max delay
    for (int i = 0; i < numDiffusers; ++i)
    {
        diffusersL[i] = DSPUtils::AllpassFilter(4096);
        diffusersR[i] = DSPUtils::AllpassFilter(4096);
    }
}

void AlgorithmicReverb::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    // Allocate pre-delay buffer (up to 500ms)
    int maxPreDelaySamples = static_cast<int>(sampleRate * 0.5);
    preDelayBufferL.resize(maxPreDelaySamples, 0.0f);
    preDelayBufferR.resize(maxPreDelaySamples, 0.0f);

    // Allocate early reflections buffer (up to 100ms)
    int maxEarlySamples = static_cast<int>(sampleRate * 0.1);
    earlyBufferL.resize(maxEarlySamples, 0.0f);
    earlyBufferR.resize(maxEarlySamples, 0.0f);

    // Allocate FDN delay lines (up to 200ms each)
    int maxFdnSamples = static_cast<int>(sampleRate * 0.2);
    for (int i = 0; i < fdnSize; ++i)
    {
        fdnDelayLines[i].resize(maxFdnSamples, 0.0f);
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

    updateParameters();
    reset();
}

void AlgorithmicReverb::reset()
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

    lfoPhase = 0.0f;
}

void AlgorithmicReverb::setMode(AlgorithmicMode newMode)
{
    mode = newMode;
    updateParameters();
}

void AlgorithmicReverb::updateParameters()
{
    const ModeSettings& settings = modeSettings[static_cast<int>(mode)];

    // Calculate pre-delay in samples
    preDelaySamples = static_cast<int>(preDelayMs * currentSampleRate / 1000.0);
    preDelaySamples = std::clamp(preDelaySamples, 0, static_cast<int>(preDelayBufferL.size()) - 1);

    // Setup early reflection delays based on mode and size
    float earlyBase = settings.earlySpacing * size * currentSampleRate / 1000.0f;
    for (int i = 0; i < numEarlyTaps; ++i)
    {
        // Fibonacci-like spacing for natural early reflections
        float delayMs = settings.earlySpacing * (i + 1) * (1.0f + size * 0.5f);
        earlyDelaysL[i] = static_cast<int>(delayMs * currentSampleRate / 1000.0f);
        earlyDelaysR[i] = static_cast<int>(delayMs * 1.1f * currentSampleRate / 1000.0f); // Slight stereo offset

        // Clamp to buffer size
        earlyDelaysL[i] = std::clamp(earlyDelaysL[i], 1, static_cast<int>(earlyBufferL.size()) - 1);
        earlyDelaysR[i] = std::clamp(earlyDelaysR[i], 1, static_cast<int>(earlyBufferR.size()) - 1);

        // Calculate gains (decay with distance)
        float tapDecay = std::pow(0.7f, static_cast<float>(i));
        earlyGainsL[i] = tapDecay * (1.0f - (i % 2) * 0.3f);
        earlyGainsR[i] = tapDecay * (1.0f - ((i + 1) % 2) * 0.3f);
    }

    // Setup FDN delay lengths based on mode and size
    // Prime-number based delays to avoid metallic resonances
    const int primeDelays[8] = { 1009, 1277, 1499, 1777, 1999, 2281, 2557, 2851 };
    float fdnScale = settings.fdnBaseDelay * size / 30.0f; // Normalize to base delay

    for (int i = 0; i < fdnSize; ++i)
    {
        float delayMs = primeDelays[i] * fdnScale / 1000.0f * std::pow(settings.fdnSpread, i * 0.5f);
        fdnDelayLengths[i] = static_cast<int>(delayMs * currentSampleRate / 1000.0f);
        fdnDelayLengths[i] = std::clamp(fdnDelayLengths[i], 1, static_cast<int>(fdnDelayLines[i].size()) - 1);
    }

    // Setup diffusers
    const int diffuserDelays[4] = { 142, 379, 573, 809 }; // Prime numbers
    for (int i = 0; i < numDiffusers; ++i)
    {
        int delay = static_cast<int>(diffuserDelays[i] * size * currentSampleRate / 44100.0);
        diffusersL[i].setDelay(std::clamp(delay, 1, 4095));
        diffusersR[i].setDelay(std::clamp(static_cast<int>(delay * 1.08f), 1, 4095)); // Stereo offset
        diffusersL[i].setFeedback(0.3f + diffusion * 0.4f);
        diffusersR[i].setFeedback(0.3f + diffusion * 0.4f);
    }

    // Update filters
    auto hpCoeffs = DSPUtils::calcHighPass(currentSampleRate, highPassFreq);
    auto lpCoeffs = DSPUtils::calcLowPass(currentSampleRate, lowPassFreq);
    highPassL.setCoefficients(hpCoeffs);
    highPassR.setCoefficients(hpCoeffs);
    lowPassL.setCoefficients(lpCoeffs);
    lowPassR.setCoefficients(lpCoeffs);

    // LFO for modulation
    lfoPhaseIncrement = modRate / static_cast<float>(currentSampleRate);
}

void AlgorithmicReverb::processEarlyReflections(float inputL, float inputR, float& outL, float& outR)
{
    // Write input to early reflection buffers
    earlyBufferL[earlyWriteIndex] = inputL;
    earlyBufferR[earlyWriteIndex] = inputR;

    outL = 0.0f;
    outR = 0.0f;

    // Sum early reflection taps
    int bufSize = static_cast<int>(earlyBufferL.size());
    for (int i = 0; i < numEarlyTaps; ++i)
    {
        int readL = earlyWriteIndex - earlyDelaysL[i];
        if (readL < 0) readL += bufSize;

        int readR = earlyWriteIndex - earlyDelaysR[i];
        if (readR < 0) readR += bufSize;

        outL += earlyBufferL[readL] * earlyGainsL[i];
        outR += earlyBufferR[readR] * earlyGainsR[i];
    }

    outL *= earlyLevel;
    outR *= earlyLevel;

    earlyWriteIndex++;
    if (earlyWriteIndex >= bufSize) earlyWriteIndex = 0;
}

void AlgorithmicReverb::processFDN(float inputL, float inputR, float& outL, float& outR)
{
    // Calculate modulation
    float lfo1 = std::sin(lfoPhase * 6.283185307179586f);
    float lfo2 = std::sin((lfoPhase + 0.25f) * 6.283185307179586f);
    lfoPhase += lfoPhaseIncrement;
    if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

    // Read from FDN delay lines
    std::array<float, fdnSize> delayOutputs;
    for (int i = 0; i < fdnSize; ++i)
    {
        int bufSize = static_cast<int>(fdnDelayLines[i].size());

        // Add modulation to delay time
        float mod = (i < 4 ? lfo1 : lfo2) * modDepth * currentSampleRate / 1000.0f;
        int modSamples = static_cast<int>(mod);

        int readIndex = fdnWriteIndices[i] - fdnDelayLengths[i] - modSamples;
        while (readIndex < 0) readIndex += bufSize;
        while (readIndex >= bufSize) readIndex -= bufSize;

        delayOutputs[i] = fdnDelayLines[i][readIndex];
    }

    // Apply Hadamard mixing matrix
    std::array<float, fdnSize> mixedOutputs;
    for (int i = 0; i < fdnSize; ++i)
    {
        mixedOutputs[i] = 0.0f;
        for (int j = 0; j < fdnSize; ++j)
        {
            mixedOutputs[i] += hadamard[i][j] * delayOutputs[j];
        }
    }

    // Calculate feedback gain from decay time
    // T60 = -60dB / (20 * log10(feedback^(samples/sampleRate)))
    // feedback = 10^(-3 / (T60 * sampleRate / avgDelay))
    float avgDelay = 0.0f;
    for (int i = 0; i < fdnSize; ++i)
        avgDelay += fdnDelayLengths[i];
    avgDelay /= fdnSize;

    float feedback;
    if (freeze || decaySeconds >= 30.0f)
    {
        feedback = 0.999f; // Nearly infinite
    }
    else
    {
        feedback = std::pow(10.0f, -3.0f * avgDelay / (decaySeconds * currentSampleRate));
        feedback = std::clamp(feedback, 0.0f, 0.999f);
    }

    // Apply damping (low-pass on feedback) and write back to delay lines
    for (int i = 0; i < fdnSize; ++i)
    {
        // Damping filter
        float dampedOutput = mixedOutputs[i] * (1.0f - damping) + fdnFilterStates[i] * damping;
        fdnFilterStates[i] = dampedOutput;

        // Add input and write to delay line
        float feedbackSample = dampedOutput * feedback;
        float newSample = (i < 4) ? inputL * 0.25f + feedbackSample : inputR * 0.25f + feedbackSample;

        int bufSize = static_cast<int>(fdnDelayLines[i].size());
        fdnDelayLines[i][fdnWriteIndices[i]] = newSample;

        fdnWriteIndices[i]++;
        if (fdnWriteIndices[i] >= bufSize) fdnWriteIndices[i] = 0;
    }

    // Sum outputs for stereo
    outL = 0.0f;
    outR = 0.0f;
    for (int i = 0; i < fdnSize; ++i)
    {
        if (i % 2 == 0)
        {
            outL += mixedOutputs[i];
            outR += mixedOutputs[i] * (1.0f - width) + mixedOutputs[(i + 1) % fdnSize] * width;
        }
        else
        {
            outR += mixedOutputs[i];
            outL += mixedOutputs[i] * (1.0f - width) + mixedOutputs[(i + 1) % fdnSize] * width;
        }
    }
    outL *= 0.25f;
    outR *= 0.25f;
}

void AlgorithmicReverb::process(juce::AudioBuffer<float>& buffer)
{
    if (bypassed)
        return;

    updateParameters();

    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    int numSamples = buffer.getNumSamples();
    int preDelayBufSize = static_cast<int>(preDelayBufferL.size());

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float inputL = leftChannel[sample];
        float inputR = rightChannel ? rightChannel[sample] : inputL;

        // Apply input high-pass filter
        inputL = highPassL.process(inputL);
        inputR = highPassR.process(inputR);

        // Pre-delay
        preDelayBufferL[preDelayWriteIndex] = inputL;
        preDelayBufferR[preDelayWriteIndex] = inputR;

        int preDelayReadIndex = preDelayWriteIndex - preDelaySamples;
        if (preDelayReadIndex < 0) preDelayReadIndex += preDelayBufSize;

        float delayedL = preDelayBufferL[preDelayReadIndex];
        float delayedR = preDelayBufferR[preDelayReadIndex];

        preDelayWriteIndex++;
        if (preDelayWriteIndex >= preDelayBufSize) preDelayWriteIndex = 0;

        // Input diffusion
        for (int i = 0; i < numDiffusers; ++i)
        {
            delayedL = diffusersL[i].process(delayedL);
            delayedR = diffusersR[i].process(delayedR);
        }

        // Early reflections
        float earlyL, earlyR;
        processEarlyReflections(delayedL, delayedR, earlyL, earlyR);

        // FDN (late reverb)
        float lateL, lateR;
        processFDN(delayedL, delayedR, lateL, lateR);

        // Combine early and late
        float wetL = earlyL + lateL;
        float wetR = earlyR + lateR;

        // Apply output low-pass filter
        wetL = lowPassL.process(wetL);
        wetR = lowPassR.process(wetR);

        // Mix dry/wet
        float dryL = leftChannel[sample];
        float dryR = rightChannel ? rightChannel[sample] : dryL;

        leftChannel[sample] = dryL * (1.0f - mix) + wetL * mix;
        if (rightChannel)
            rightChannel[sample] = dryR * (1.0f - mix) + wetR * mix;
    }
}
