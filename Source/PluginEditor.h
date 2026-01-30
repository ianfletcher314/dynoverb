#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/LookAndFeel.h"

class DynoverbAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      public juce::Timer
{
public:
    DynoverbAudioProcessorEditor(DynoverbAudioProcessor&);
    ~DynoverbAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    DynoverbAudioProcessor& audioProcessor;

    // Custom LookAndFeel
    std::unique_ptr<DynoverbLookAndFeel> lookAndFeel;

    // Type selector
    juce::ComboBox typeSelector;
    juce::Label typeLabel;

    // Type-specific selectors
    juce::ComboBox algoModeSelector;
    juce::Label algoModeLabel;

    juce::ComboBox shimmerPitchSelector;
    juce::Label shimmerPitchLabel;

    // Global controls - main row
    juce::Slider preDelaySlider;
    juce::Label preDelayLabel;
    juce::ToggleButton preDelayTempoSyncButton;
    juce::ComboBox preDelaySyncDivSelector;

    juce::Slider decaySlider;
    juce::Label decayLabel;

    juce::Slider dampingSlider;
    juce::Label dampingLabel;

    juce::Slider sizeSlider;
    juce::Label sizeLabel;

    juce::Slider diffusionSlider;
    juce::Label diffusionLabel;

    // Global controls - second row
    juce::Slider modRateSlider;
    juce::Label modRateLabel;

    juce::Slider modDepthSlider;
    juce::Label modDepthLabel;

    juce::Slider earlyLevelSlider;
    juce::Label earlyLevelLabel;

    juce::Slider widthSlider;
    juce::Label widthLabel;

    // Filter controls
    juce::Slider highPassSlider;
    juce::Label highPassLabel;

    juce::Slider lowPassSlider;
    juce::Label lowPassLabel;

    // Output controls
    juce::Slider duckingSlider;
    juce::Label duckingLabel;

    juce::Slider mixSlider;
    juce::Label mixLabel;

    // Type-specific controls - Shimmer
    juce::Slider shimmerAmountSlider;
    juce::Label shimmerAmountLabel;
    juce::ToggleButton shimmerInfiniteButton;

    // Type-specific controls - Spring
    juce::Slider springTensionSlider;
    juce::Label springTensionLabel;
    juce::Slider springDripSlider;
    juce::Label springDripLabel;
    juce::Slider springMixSlider;
    juce::Label springMixLabel;

    // Type-specific controls - Gated
    juce::Slider gateThresholdSlider;
    juce::Label gateThresholdLabel;
    juce::Slider gateHoldSlider;
    juce::Label gateHoldLabel;
    juce::Slider gateReleaseSlider;
    juce::Label gateReleaseLabel;
    juce::Slider gateShapeSlider;
    juce::Label gateShapeLabel;

    // Buttons
    juce::ToggleButton freezeButton;
    juce::ToggleButton bypassButton;

    // Meters and visualizers
    LevelMeter inputMeter;
    LevelMeter outputMeter;
    GateMeter gateMeter;
    DecayVisualizer decayVisualizer;

    // Smoothed metering values
    float smoothedInputLevel = 0.0f;
    float smoothedOutputLevel = 0.0f;

    // APVTS Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> algoModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> shimmerPitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> preDelaySyncDivAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preDelayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> preDelayTempoSyncAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> diffusionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> earlyLevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highPassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowPassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> duckingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> shimmerAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> shimmerInfiniteAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> springTensionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> springDripAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> springMixAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gateThresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gateHoldAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gateReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gateShapeAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> freezeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    // Helper to show/hide type-specific controls
    void updateVisibleControls();

    // Setup helper methods
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText);
    void setupComboBox(juce::ComboBox& comboBox, juce::Label& label, const juce::String& labelText, const juce::StringArray& items);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynoverbAudioProcessorEditor)
};
