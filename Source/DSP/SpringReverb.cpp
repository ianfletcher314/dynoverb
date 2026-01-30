#include "SpringReverb.h"
#include <cmath>

SpringReverb::SpringReverb()
{
    // Initialize diffusers
    for (int i = 0; i < numDiffusers; ++i)
    {
        diffusersL[i] = DSPUtils::AllpassFilter(2048);
        diffusersR[i] = DSPUtils::AllpassFilter(2048);
    }
}

void SpringReverb::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    // Allocate pre-delay buffer
    int maxPreDelaySamples = static_cast<int>(sampleRate * 0.5);
    preDelayBufferL.resize(maxPreDelaySamples, 0.0f);
    preDelayBufferR.resize(maxPreDelaySamples, 0.0f);

    // Initialize spring delay lines
    int maxDelaySamples = static_cast<int>(sampleRate * 0.15);
    for (int s = 0; s < numSprings; ++s)
    {
        springsL[s].delayLine.resize(maxDelaySamples, 0.0f);
        springsL[s].allpassBuffer.resize(1024, 0.0f);
        springsL[s].delayLength = static_cast<int>(baseDelayLengths[s] * sampleRate / 44100.0);
        springsL[s].writeIndex = 0;
        springsL[s].allpassWriteIndex = 0;
        springsL[s].filterState = 0.0f;
        springsL[s].allpassState = 0.0f;
        springsL[s].chirpState1 = 0.0f;
        springsL[s].chirpState2 = 0.0f;

        springsR[s].delayLine.resize(maxDelaySamples, 0.0f);
        springsR[s].allpassBuffer.resize(1024, 0.0f);
        // Slight stereo offset
        springsR[s].delayLength = static_cast<int>(baseDelayLengths[s] * 1.07f * sampleRate / 44100.0);
        springsR[s].writeIndex = 0;
        springsR[s].allpassWriteIndex = 0;
        springsR[s].filterState = 0.0f;
        springsR[s].allpassState = 0.0f;
        springsR[s].chirpState1 = 0.0f;
        springsR[s].chirpState2 = 0.0f;
    }

    // Setup filters
    auto hpCoeffs = DSPUtils::calcHighPass(sampleRate, highPassFreq);
    auto lpCoeffs = DSPUtils::calcLowPass(sampleRate, lowPassFreq);
    highPassL.setCoefficients(hpCoeffs);
    highPassR.setCoefficients(hpCoeffs);
    lowPassL.setCoefficients(lpCoeffs);
    lowPassR.setCoefficients(lpCoeffs);

    // Spring response is band-limited
    springLPL.setCutoff(sampleRate, 4000.0f);
    springLPR.setCutoff(sampleRate, 4000.0f);

    updateParameters();
    reset();
}

void SpringReverb::reset()
{
    std::fill(preDelayBufferL.begin(), preDelayBufferL.end(), 0.0f);
    std::fill(preDelayBufferR.begin(), preDelayBufferR.end(), 0.0f);
    preDelayWriteIndex = 0;

    for (int s = 0; s < numSprings; ++s)
    {
        std::fill(springsL[s].delayLine.begin(), springsL[s].delayLine.end(), 0.0f);
        std::fill(springsL[s].allpassBuffer.begin(), springsL[s].allpassBuffer.end(), 0.0f);
        springsL[s].writeIndex = 0;
        springsL[s].allpassWriteIndex = 0;
        springsL[s].filterState = 0.0f;
        springsL[s].allpassState = 0.0f;
        springsL[s].chirpState1 = 0.0f;
        springsL[s].chirpState2 = 0.0f;

        std::fill(springsR[s].delayLine.begin(), springsR[s].delayLine.end(), 0.0f);
        std::fill(springsR[s].allpassBuffer.begin(), springsR[s].allpassBuffer.end(), 0.0f);
        springsR[s].writeIndex = 0;
        springsR[s].allpassWriteIndex = 0;
        springsR[s].filterState = 0.0f;
        springsR[s].allpassState = 0.0f;
        springsR[s].chirpState1 = 0.0f;
        springsR[s].chirpState2 = 0.0f;
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
    springLPL.reset();
    springLPR.reset();

    dripPhase = 0.0f;
    dripNoise = 0.0f;
}

void SpringReverb::setTension(float tensionAmount)
{
    tension = std::clamp(tensionAmount, 0.0f, 1.0f);
    updateParameters();
}

void SpringReverb::setDrip(float dripAmount)
{
    drip = std::clamp(dripAmount, 0.0f, 1.0f);
}

void SpringReverb::setSpringMix(float springAmount)
{
    springMix = std::clamp(springAmount, 0.0f, 1.0f);
}

void SpringReverb::updateParameters()
{
    // Calculate pre-delay
    preDelaySamples = static_cast<int>(preDelayMs * currentSampleRate / 1000.0);
    preDelaySamples = std::clamp(preDelaySamples, 0, static_cast<int>(preDelayBufferL.size()) - 1);

    // Update spring delay lengths based on size and tension
    int maxDelaySamples = static_cast<int>(springsL[0].delayLine.size()) - 1;
    for (int s = 0; s < numSprings; ++s)
    {
        float tensionFactor = 0.7f + tension * 0.6f;  // Higher tension = shorter delay (higher pitch)
        float sizeFactor = 0.5f + size * 1.0f;

        springsL[s].delayLength = static_cast<int>(baseDelayLengths[s] * sizeFactor / tensionFactor *
                                                   currentSampleRate / 44100.0);
        springsL[s].delayLength = std::clamp(springsL[s].delayLength, 100, maxDelaySamples);

        springsR[s].delayLength = static_cast<int>(baseDelayLengths[s] * 1.07f * sizeFactor / tensionFactor *
                                                   currentSampleRate / 44100.0);
        springsR[s].delayLength = std::clamp(springsR[s].delayLength, 100, maxDelaySamples);
    }

    // Update diffusers
    const int diffuserDelays[3] = { 97, 211, 367 };
    for (int i = 0; i < numDiffusers; ++i)
    {
        int delay = static_cast<int>(diffuserDelays[i] * size * currentSampleRate / 44100.0);
        diffusersL[i].setDelay(std::clamp(delay, 1, 2047));
        diffusersR[i].setDelay(std::clamp(static_cast<int>(delay * 1.1f), 1, 2047));
        diffusersL[i].setFeedback(0.3f + diffusion * 0.35f);
        diffusersR[i].setFeedback(0.3f + diffusion * 0.35f);
    }

    // Update filters
    auto hpCoeffs = DSPUtils::calcHighPass(currentSampleRate, highPassFreq);
    auto lpCoeffs = DSPUtils::calcLowPass(currentSampleRate, lowPassFreq);
    highPassL.setCoefficients(hpCoeffs);
    highPassR.setCoefficients(hpCoeffs);
    lowPassL.setCoefficients(lpCoeffs);
    lowPassR.setCoefficients(lpCoeffs);

    // Spring characteristic frequency based on tension
    float springCutoff = 2000.0f + tension * 3000.0f;
    springLPL.setCutoff(currentSampleRate, springCutoff);
    springLPR.setCutoff(currentSampleRate, springCutoff);
}

float SpringReverb::processSpringModel(float input, int channel)
{
    auto& springs = (channel == 0) ? springsL : springsR;
    float output = 0.0f;

    // Calculate feedback coefficient from decay
    float feedback;
    if (freeze || decaySeconds >= 30.0f)
    {
        feedback = 0.995f;
    }
    else
    {
        float avgDelay = 0.0f;
        for (int s = 0; s < numSprings; ++s)
            avgDelay += springs[s].delayLength;
        avgDelay /= numSprings;

        feedback = std::pow(10.0f, -3.0f * avgDelay / (decaySeconds * currentSampleRate));
        feedback = std::clamp(feedback, 0.0f, 0.98f);
    }

    for (int s = 0; s < numSprings; ++s)
    {
        SpringDelay& spring = springs[s];
        int bufSize = static_cast<int>(spring.delayLine.size());
        int apBufSize = static_cast<int>(spring.allpassBuffer.size());

        // Read from delay line
        int readIndex = spring.writeIndex - spring.delayLength;
        if (readIndex < 0) readIndex += bufSize;

        float delayed = spring.delayLine[readIndex];

        // Spring characteristic: chirp filter (dispersion)
        // High frequencies arrive slightly before low frequencies
        // Implement as cascaded allpass filters with frequency-dependent delay
        float chirpCoeff = 0.3f + tension * 0.4f;

        // First order allpass for chirp
        float chirpOut1 = chirpCoeff * (delayed - spring.chirpState1) + spring.delayLine[
            (readIndex + spring.delayLength / 4) % bufSize];
        spring.chirpState1 = chirpOut1;

        float chirpOut2 = chirpCoeff * (chirpOut1 - spring.chirpState2) + delayed;
        spring.chirpState2 = chirpOut2;

        // Blend between chirped and normal based on tension
        float springOut = delayed * (1.0f - tension * 0.5f) + chirpOut2 * tension * 0.5f;

        // Damping filter
        spring.filterState = springOut * (1.0f - damping) + spring.filterState * damping;

        // Drip effect: occasional random modulation simulating spring chaos
        float dripMod = 1.0f;
        if (drip > 0.0f)
        {
            // Simple drip: randomly modulate feedback occasionally
            if (random.nextFloat() < drip * 0.001f)
            {
                dripNoise = random.nextFloat() * 2.0f - 1.0f;
            }
            dripNoise *= 0.995f;  // Decay
            dripMod = 1.0f + dripNoise * drip * 0.3f;
        }

        // Write new sample with feedback
        float feedbackSample = spring.filterState * feedback * dripMod;
        spring.delayLine[spring.writeIndex] = input * (1.0f / numSprings) + feedbackSample;

        spring.writeIndex++;
        if (spring.writeIndex >= bufSize) spring.writeIndex = 0;

        // Accumulate output with different gains for each spring
        float springGain = 1.0f - s * 0.2f;  // Slightly reduce gain for secondary springs
        output += spring.filterState * springGain;
    }

    return output / numSprings;
}

void SpringReverb::process(juce::AudioBuffer<float>& buffer)
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

        // Input filtering
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

        // Diffusion (before spring)
        for (int i = 0; i < numDiffusers; ++i)
        {
            delayedL = diffusersL[i].process(delayedL);
            delayedR = diffusersR[i].process(delayedR);
        }

        // Process through spring model
        float springOutL = processSpringModel(delayedL, 0);
        float springOutR = processSpringModel(delayedR, 1);

        // Apply spring low-pass characteristic (springs have limited bandwidth)
        springOutL = springLPL.process(springOutL);
        springOutR = springLPR.process(springOutR);

        // Blend spring and diffused signal based on springMix
        float wetL = delayedL * (1.0f - springMix) + springOutL * springMix;
        float wetR = delayedR * (1.0f - springMix) + springOutR * springMix;

        // Output filtering
        wetL = lowPassL.process(wetL);
        wetR = lowPassR.process(wetR);

        // Apply width
        float mid = (wetL + wetR) * 0.5f;
        float side = (wetL - wetR) * 0.5f;
        wetL = mid + side * width;
        wetR = mid - side * width;

        // Mix
        float dryL = leftChannel[sample];
        float dryR = rightChannel ? rightChannel[sample] : dryL;

        leftChannel[sample] = dryL * (1.0f - mix) + wetL * mix;
        if (rightChannel)
            rightChannel[sample] = dryR * (1.0f - mix) + wetR * mix;
    }
}
