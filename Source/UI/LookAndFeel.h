#pragma once

#include <JuceHeader.h>

namespace Colors
{
    const juce::Colour background    = juce::Colour(0xff1a1a1a);
    const juce::Colour panelBg       = juce::Colour(0xff252525);
    const juce::Colour panelBorder   = juce::Colour(0xff353535);
    const juce::Colour accent        = juce::Colour(0xff6b8cff);  // Blue-ish purple for reverb
    const juce::Colour accentAlt     = juce::Colour(0xff9b6bff);  // Purple accent
    const juce::Colour textPrimary   = juce::Colour(0xfff0f0f0);
    const juce::Colour textSecondary = juce::Colour(0xff909090);
    const juce::Colour meterGreen    = juce::Colour(0xff22c55e);
    const juce::Colour meterYellow   = juce::Colour(0xffeab308);
    const juce::Colour meterRed      = juce::Colour(0xffef4444);
    const juce::Colour knobBody      = juce::Colour(0xff404040);
    const juce::Colour led           = juce::Colour(0xff6b8cff);
    const juce::Colour ledOff        = juce::Colour(0xff303050);
    const juce::Colour freeze        = juce::Colour(0xff00d4ff);  // Cyan for freeze
}

class DynoverbLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DynoverbLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, Colors::accent);
        setColour(juce::Slider::rotarySliderFillColourId, Colors::accent);
        setColour(juce::Slider::rotarySliderOutlineColourId, Colors::panelBorder);
        setColour(juce::Label::textColourId, Colors::textPrimary);
        setColour(juce::ComboBox::backgroundColourId, Colors::panelBg);
        setColour(juce::ComboBox::textColourId, Colors::textPrimary);
        setColour(juce::ComboBox::outlineColourId, Colors::panelBorder);
        setColour(juce::ComboBox::arrowColourId, Colors::textSecondary);
        setColour(juce::PopupMenu::backgroundColourId, Colors::panelBg);
        setColour(juce::PopupMenu::textColourId, Colors::textPrimary);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, Colors::accent);
        setColour(juce::ToggleButton::textColourId, Colors::textPrimary);
        setColour(juce::ToggleButton::tickColourId, Colors::accent);
    }

    void setAccentColour(juce::Colour c) { accentColour = c; }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float, float,
                          juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(2.0f);
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;

        // Outer ring (knurled edge)
        g.setColour(juce::Colour(0xff303030));
        g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        // Knurl pattern
        g.setColour(juce::Colour(0xff404040));
        int numKnurls = 24;
        for (int i = 0; i < numKnurls; ++i)
        {
            float angle = i * juce::MathConstants<float>::twoPi / numKnurls;
            float x1 = cx + (radius - 1.0f) * std::cos(angle);
            float y1 = cy + (radius - 1.0f) * std::sin(angle);
            float x2 = cx + (radius - 4.0f) * std::cos(angle);
            float y2 = cy + (radius - 4.0f) * std::sin(angle);
            g.drawLine(x1, y1, x2, y2, 1.5f);
        }

        // Main knob body with gradient
        float innerRadius = radius * 0.78f;
        juce::ColourGradient knobGradient(juce::Colour(0xff555555), cx - innerRadius * 0.5f, cy - innerRadius * 0.5f,
                                           juce::Colour(0xff252525), cx + innerRadius * 0.5f, cy + innerRadius * 0.5f, true);
        g.setGradientFill(knobGradient);
        g.fillEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

        // Ring
        g.setColour(juce::Colour(0xff606060));
        g.drawEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f, 1.0f);

        // Arc showing value
        float startAngle = juce::MathConstants<float>::pi * 1.25f;
        float endAngle = juce::MathConstants<float>::pi * 2.75f;
        float valueAngle = startAngle + sliderPosProportional * (endAngle - startAngle);

        juce::Path arcPath;
        arcPath.addArc(cx - radius + 3.0f, cy - radius + 3.0f,
                       (radius - 3.0f) * 2.0f, (radius - 3.0f) * 2.0f,
                       startAngle, valueAngle, true);
        g.setColour(accentColour.withAlpha(0.8f));
        g.strokePath(arcPath, juce::PathStrokeType(3.0f));

        // Indicator line (7 o'clock to 5 o'clock range)
        float indicatorAngle = juce::jmap(sliderPosProportional, 0.0f, 1.0f, -1.047f, 4.189f) + juce::MathConstants<float>::pi;
        float indicatorLength = innerRadius * 0.65f;
        float ix1 = cx + (innerRadius * 0.2f) * std::cos(indicatorAngle);
        float iy1 = cy + (innerRadius * 0.2f) * std::sin(indicatorAngle);
        float ix2 = cx + indicatorLength * std::cos(indicatorAngle);
        float iy2 = cy + indicatorLength * std::sin(indicatorAngle);
        g.setColour(accentColour);
        g.drawLine(ix1, iy1, ix2, iy2, 3.0f);

        // Center cap
        float capRadius = innerRadius * 0.25f;
        g.setColour(juce::Colour(0xff404040));
        g.fillEllipse(cx - capRadius, cy - capRadius, capRadius * 2.0f, capRadius * 2.0f);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (style == juce::Slider::LinearHorizontal)
        {
            auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height);

            // Track background
            float trackHeight = 4.0f;
            auto trackBounds = bounds.withSizeKeepingCentre(bounds.getWidth() - 4.0f, trackHeight);
            g.setColour(Colors::panelBorder);
            g.fillRoundedRectangle(trackBounds, 2.0f);

            // Filled portion
            float fillWidth = sliderPos - minSliderPos;
            auto fillBounds = trackBounds.withWidth(fillWidth);
            g.setColour(accentColour);
            g.fillRoundedRectangle(fillBounds, 2.0f);

            // Thumb
            float thumbRadius = 8.0f;
            g.setColour(Colors::textPrimary);
            g.fillEllipse(sliderPos - thumbRadius, bounds.getCentreY() - thumbRadius, thumbRadius * 2.0f, thumbRadius * 2.0f);
        }
        else
        {
            LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);

        g.setColour(Colors::panelBg);
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(isButtonDown ? Colors::accent : Colors::panelBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // Arrow
        juce::Path arrow;
        float arrowX = buttonX + buttonW * 0.5f;
        float arrowY = buttonY + buttonH * 0.5f;
        arrow.addTriangle(arrowX - 4.0f, arrowY - 2.0f,
                          arrowX + 4.0f, arrowY - 2.0f,
                          arrowX, arrowY + 3.0f);
        g.setColour(Colors::textSecondary);
        g.fillPath(arrow);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        bool isOn = button.getToggleState();

        // LED-style button
        float ledSize = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.4f;
        float ledX = bounds.getX() + 4.0f;
        float ledY = bounds.getCentreY() - ledSize * 0.5f;

        // LED background
        g.setColour(isOn ? accentColour : Colors::ledOff);
        g.fillEllipse(ledX, ledY, ledSize, ledSize);

        // LED glow when on
        if (isOn)
        {
            g.setColour(accentColour.withAlpha(0.3f));
            g.fillEllipse(ledX - 2.0f, ledY - 2.0f, ledSize + 4.0f, ledSize + 4.0f);
        }

        // Text
        g.setColour(Colors::textPrimary);
        g.setFont(juce::Font(12.0f));
        g.drawText(button.getButtonText(),
                   bounds.withLeft(ledX + ledSize + 6.0f),
                   juce::Justification::centredLeft);
    }

private:
    juce::Colour accentColour = Colors::accent;
};

// Level meter component
class LevelMeter : public juce::Component
{
public:
    void setLevel(float newLevel) { level = newLevel; repaint(); }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);

        // Background
        g.setColour(juce::Colour(0xff151515));
        g.fillRoundedRectangle(bounds, 3.0f);

        // Convert to dB and normalize
        float db = juce::Decibels::gainToDecibels(level, -60.0f);
        float normalized = juce::jmap(db, -60.0f, 0.0f, 0.0f, 1.0f);
        normalized = juce::jlimit(0.0f, 1.0f, normalized);

        // Level bar (vertical)
        float barHeight = bounds.getHeight() * normalized;

        juce::Colour barColour;
        if (normalized < 0.6f)
            barColour = Colors::meterGreen;
        else if (normalized < 0.85f)
            barColour = Colors::meterYellow;
        else
            barColour = Colors::meterRed;

        auto barBounds = bounds.removeFromBottom(barHeight);
        g.setColour(barColour);
        g.fillRoundedRectangle(barBounds, 2.0f);
    }

private:
    float level = 0.0f;
};

// Gate level indicator for gated reverb
class GateMeter : public juce::Component
{
public:
    void setGateLevel(float newLevel) { gateLevel = newLevel; repaint(); }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);

        // Background
        g.setColour(juce::Colour(0xff151515));
        g.fillRoundedRectangle(bounds, 3.0f);

        // Gate level (horizontal bar)
        float barWidth = bounds.getWidth() * gateLevel;
        auto barBounds = bounds.withWidth(barWidth);

        // Color gradient based on level
        juce::Colour barColour = Colors::accent.interpolatedWith(Colors::freeze, gateLevel);
        g.setColour(barColour);
        g.fillRoundedRectangle(barBounds, 2.0f);

        // Border
        g.setColour(Colors::panelBorder);
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
    }

private:
    float gateLevel = 0.0f;
};

// Decay envelope visualization
class DecayVisualizer : public juce::Component
{
public:
    void setDecay(float decaySeconds, bool frozen)
    {
        this->decaySeconds = decaySeconds;
        this->frozen = frozen;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(4.0f);

        // Background
        g.setColour(Colors::panelBg);
        g.fillRoundedRectangle(bounds, 4.0f);

        // Draw decay curve
        juce::Path decayCurve;
        float startX = bounds.getX();
        float endX = bounds.getRight();
        float topY = bounds.getY() + 4.0f;
        float bottomY = bounds.getBottom() - 4.0f;
        float range = bottomY - topY;

        decayCurve.startNewSubPath(startX, topY);

        if (frozen || decaySeconds >= 30.0f)
        {
            // Infinite - flat line
            decayCurve.lineTo(endX, topY);
        }
        else
        {
            // Exponential decay
            float numPoints = bounds.getWidth();
            for (float i = 1; i <= numPoints; ++i)
            {
                float t = i / numPoints;
                float decay = std::exp(-3.0f * t / std::max(0.1f, decaySeconds / 5.0f));
                float y = topY + range * (1.0f - decay);
                decayCurve.lineTo(startX + t * (endX - startX), y);
            }
        }

        g.setColour(frozen ? Colors::freeze : Colors::accent);
        g.strokePath(decayCurve, juce::PathStrokeType(2.0f));

        // Border
        g.setColour(Colors::panelBorder);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    }

private:
    float decaySeconds = 2.0f;
    bool frozen = false;
};
