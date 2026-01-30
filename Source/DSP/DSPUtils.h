#pragma once

#include <cmath>
#include <algorithm>

namespace DSPUtils
{
    // Level conversion
    inline float linearToDecibels(float linear)
    {
        return linear > 0.0f ? 20.0f * std::log10(linear) : -100.0f;
    }

    inline float decibelsToLinear(float dB)
    {
        return std::pow(10.0f, dB / 20.0f);
    }

    // Range mapping
    inline float mapRange(float value, float inMin, float inMax, float outMin, float outMax)
    {
        return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
    }

    // Clipping functions
    inline float softClip(float sample)
    {
        return std::tanh(sample);
    }

    inline float hardClip(float sample, float threshold = 1.0f)
    {
        return std::clamp(sample, -threshold, threshold);
    }

    // One-pole filter coefficient
    inline float calculateCoefficient(double sampleRate, float timeMs)
    {
        if (timeMs <= 0.0f) return 1.0f;
        return 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * timeMs * 0.001f));
    }

    // Allpass filter for reverb
    class AllpassFilter
    {
    public:
        AllpassFilter(int maxDelay = 8192) : maxDelayLength(maxDelay)
        {
            buffer.resize(maxDelay, 0.0f);
        }

        void setDelay(int delaySamples)
        {
            delay = std::clamp(delaySamples, 1, maxDelayLength - 1);
        }

        void setFeedback(float fb)
        {
            feedback = std::clamp(fb, 0.0f, 0.99f);
        }

        void reset()
        {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
            writeIndex = 0;
        }

        float process(float input)
        {
            int readIndex = writeIndex - delay;
            if (readIndex < 0) readIndex += maxDelayLength;

            float delayed = buffer[readIndex];
            float output = -input + delayed;
            buffer[writeIndex] = input + delayed * feedback;

            writeIndex++;
            if (writeIndex >= maxDelayLength) writeIndex = 0;

            return output;
        }

    private:
        std::vector<float> buffer;
        int maxDelayLength;
        int delay = 100;
        int writeIndex = 0;
        float feedback = 0.5f;
    };

    // Comb filter for reverb
    class CombFilter
    {
    public:
        CombFilter(int maxDelay = 8192) : maxDelayLength(maxDelay)
        {
            buffer.resize(maxDelay, 0.0f);
        }

        void setDelay(int delaySamples)
        {
            delay = std::clamp(delaySamples, 1, maxDelayLength - 1);
        }

        void setFeedback(float fb)
        {
            feedback = std::clamp(fb, 0.0f, 0.99f);
        }

        void setDamping(float damp)
        {
            damping = std::clamp(damp, 0.0f, 1.0f);
        }

        void reset()
        {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
            writeIndex = 0;
            filterState = 0.0f;
        }

        float process(float input)
        {
            int readIndex = writeIndex - delay;
            if (readIndex < 0) readIndex += maxDelayLength;

            float output = buffer[readIndex];

            // Low-pass damping filter
            filterState = output * (1.0f - damping) + filterState * damping;

            buffer[writeIndex] = input + filterState * feedback;

            writeIndex++;
            if (writeIndex >= maxDelayLength) writeIndex = 0;

            return output;
        }

    private:
        std::vector<float> buffer;
        int maxDelayLength;
        int delay = 1000;
        int writeIndex = 0;
        float feedback = 0.8f;
        float damping = 0.5f;
        float filterState = 0.0f;
    };

    // Modulated delay line for chorus/shimmer effects
    class ModulatedDelayLine
    {
    public:
        ModulatedDelayLine(int maxDelay = 48000) : maxDelayLength(maxDelay)
        {
            buffer.resize(maxDelay, 0.0f);
        }

        void setDelay(float delaySamples)
        {
            baseDelay = std::clamp(delaySamples, 1.0f, static_cast<float>(maxDelayLength - 2));
        }

        void setModDepth(float depth)
        {
            modDepth = depth;
        }

        void reset()
        {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
            writeIndex = 0;
        }

        float process(float input, float modulation)
        {
            // Write to buffer
            buffer[writeIndex] = input;

            // Calculate modulated read position
            float readPos = static_cast<float>(writeIndex) - baseDelay - modulation * modDepth;
            while (readPos < 0) readPos += maxDelayLength;
            while (readPos >= maxDelayLength) readPos -= maxDelayLength;

            // Linear interpolation
            int readIndex1 = static_cast<int>(readPos);
            int readIndex2 = (readIndex1 + 1) % maxDelayLength;
            float frac = readPos - static_cast<float>(readIndex1);

            float output = buffer[readIndex1] * (1.0f - frac) + buffer[readIndex2] * frac;

            writeIndex++;
            if (writeIndex >= maxDelayLength) writeIndex = 0;

            return output;
        }

    private:
        std::vector<float> buffer;
        int maxDelayLength;
        float baseDelay = 1000.0f;
        float modDepth = 10.0f;
        int writeIndex = 0;
    };

    // Simple one-pole low-pass filter
    class OnePoleFilter
    {
    public:
        void setCoefficient(float coeff)
        {
            a0 = std::clamp(coeff, 0.0f, 1.0f);
            b1 = 1.0f - a0;
        }

        void setCutoff(double sampleRate, float freqHz)
        {
            float w = 2.0f * 3.14159265358979323846f * freqHz / static_cast<float>(sampleRate);
            a0 = w / (1.0f + w);
            b1 = 1.0f - a0;
        }

        void reset()
        {
            z1 = 0.0f;
        }

        float process(float input)
        {
            z1 = input * a0 + z1 * b1;
            return z1;
        }

    private:
        float a0 = 0.1f;
        float b1 = 0.9f;
        float z1 = 0.0f;
    };

    // One-pole high-pass filter
    class OnePoleLPHPFilter
    {
    public:
        void setHighPass(double sampleRate, float freqHz)
        {
            float rc = 1.0f / (2.0f * 3.14159265358979323846f * freqHz);
            float dt = 1.0f / static_cast<float>(sampleRate);
            alpha = rc / (rc + dt);
            isHighPass = true;
        }

        void setLowPass(double sampleRate, float freqHz)
        {
            float rc = 1.0f / (2.0f * 3.14159265358979323846f * freqHz);
            float dt = 1.0f / static_cast<float>(sampleRate);
            alpha = dt / (rc + dt);
            isHighPass = false;
        }

        void reset()
        {
            prevInput = 0.0f;
            prevOutput = 0.0f;
        }

        float process(float input)
        {
            float output;
            if (isHighPass)
            {
                output = alpha * (prevOutput + input - prevInput);
            }
            else
            {
                output = prevOutput + alpha * (input - prevOutput);
            }
            prevInput = input;
            prevOutput = output;
            return output;
        }

    private:
        float alpha = 0.5f;
        float prevInput = 0.0f;
        float prevOutput = 0.0f;
        bool isHighPass = true;
    };

    // Biquad filter structure and calculator
    struct BiquadCoeffs
    {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
    };

    inline BiquadCoeffs calcLowPass(double sampleRate, float freq, float q = 0.707f)
    {
        BiquadCoeffs c;
        float w0 = 2.0f * 3.14159265358979323846f * freq / static_cast<float>(sampleRate);
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.0f * q);

        float a0 = 1.0f + alpha;
        c.b0 = ((1.0f - cosw0) / 2.0f) / a0;
        c.b1 = (1.0f - cosw0) / a0;
        c.b2 = c.b0;
        c.a1 = (-2.0f * cosw0) / a0;
        c.a2 = (1.0f - alpha) / a0;

        return c;
    }

    inline BiquadCoeffs calcHighPass(double sampleRate, float freq, float q = 0.707f)
    {
        BiquadCoeffs c;
        float w0 = 2.0f * 3.14159265358979323846f * freq / static_cast<float>(sampleRate);
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.0f * q);

        float a0 = 1.0f + alpha;
        c.b0 = ((1.0f + cosw0) / 2.0f) / a0;
        c.b1 = -(1.0f + cosw0) / a0;
        c.b2 = c.b0;
        c.a1 = (-2.0f * cosw0) / a0;
        c.a2 = (1.0f - alpha) / a0;

        return c;
    }

    // Biquad processor
    class BiquadFilter
    {
    public:
        void setCoefficients(const BiquadCoeffs& coeffs)
        {
            c = coeffs;
        }

        void reset()
        {
            x1 = x2 = y1 = y2 = 0.0f;
        }

        float process(float input)
        {
            float output = c.b0 * input + c.b1 * x1 + c.b2 * x2 - c.a1 * y1 - c.a2 * y2;
            x2 = x1;
            x1 = input;
            y2 = y1;
            y1 = output;
            return output;
        }

    private:
        BiquadCoeffs c;
        float x1 = 0.0f, x2 = 0.0f;
        float y1 = 0.0f, y2 = 0.0f;
    };

    // Envelope follower for ducking
    class EnvelopeFollower
    {
    public:
        void setAttack(double sampleRate, float attackMs)
        {
            attackCoeff = calculateCoefficient(sampleRate, attackMs);
        }

        void setRelease(double sampleRate, float releaseMs)
        {
            releaseCoeff = calculateCoefficient(sampleRate, releaseMs);
        }

        void reset()
        {
            envelope = 0.0f;
        }

        float process(float input)
        {
            float inputAbs = std::abs(input);
            if (inputAbs > envelope)
                envelope += attackCoeff * (inputAbs - envelope);
            else
                envelope += releaseCoeff * (inputAbs - envelope);
            return envelope;
        }

    private:
        float attackCoeff = 0.1f;
        float releaseCoeff = 0.01f;
        float envelope = 0.0f;
    };
}
