#include "PluginProcessor.h"
#include "PluginEditor.h"

DynoverbAudioProcessorEditor::DynoverbAudioProcessorEditor(DynoverbAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    lookAndFeel = std::make_unique<DynoverbLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());

    // Type selector
    setupComboBox(typeSelector, typeLabel, "TYPE",
                  juce::StringArray{ "Algorithmic", "Shimmer", "Spring", "Gated" });
    typeSelector.onChange = [this]() { updateVisibleControls(); };

    // Algo mode selector
    setupComboBox(algoModeSelector, algoModeLabel, "MODE",
                  juce::StringArray{ "Room", "Hall", "Plate", "Chamber" });

    // Shimmer pitch selector
    setupComboBox(shimmerPitchSelector, shimmerPitchLabel, "PITCH",
                  juce::StringArray{ "Oct Up", "5th Up", "Oct Down", "5th Down", "Mixed" });

    // Pre-delay sync division
    setupComboBox(preDelaySyncDivSelector, preDelayLabel, "",
                  juce::StringArray{ "1/32", "1/16T", "1/16", "1/8T", "1/8", "1/4T", "1/4", "1/2", "1/1" });
    preDelaySyncDivSelector.setVisible(false);

    // Global controls
    setupSlider(preDelaySlider, preDelayLabel, "PRE-DELAY");
    setupSlider(decaySlider, decayLabel, "DECAY");
    setupSlider(dampingSlider, dampingLabel, "DAMPING");
    setupSlider(sizeSlider, sizeLabel, "SIZE");
    setupSlider(diffusionSlider, diffusionLabel, "DIFFUSION");

    setupSlider(modRateSlider, modRateLabel, "MOD RATE");
    setupSlider(modDepthSlider, modDepthLabel, "MOD DEPTH");
    setupSlider(earlyLevelSlider, earlyLevelLabel, "EARLY");
    setupSlider(widthSlider, widthLabel, "WIDTH");

    setupSlider(highPassSlider, highPassLabel, "HIGH PASS");
    setupSlider(lowPassSlider, lowPassLabel, "LOW PASS");
    setupSlider(duckingSlider, duckingLabel, "DUCKING");
    setupSlider(mixSlider, mixLabel, "MIX");

    // Shimmer controls
    setupSlider(shimmerAmountSlider, shimmerAmountLabel, "SHIMMER");

    shimmerInfiniteButton.setButtonText("Infinite");
    addAndMakeVisible(shimmerInfiniteButton);

    // Spring controls
    setupSlider(springTensionSlider, springTensionLabel, "TENSION");
    setupSlider(springDripSlider, springDripLabel, "DRIP");
    setupSlider(springMixSlider, springMixLabel, "SPRING MIX");

    // Gated controls
    setupSlider(gateThresholdSlider, gateThresholdLabel, "THRESHOLD");
    setupSlider(gateHoldSlider, gateHoldLabel, "HOLD");
    setupSlider(gateReleaseSlider, gateReleaseLabel, "RELEASE");
    setupSlider(gateShapeSlider, gateShapeLabel, "SHAPE");

    // Tempo sync button
    preDelayTempoSyncButton.setButtonText("Sync");
    preDelayTempoSyncButton.onClick = [this]()
    {
        bool synced = preDelayTempoSyncButton.getToggleState();
        preDelaySlider.setVisible(!synced);
        preDelaySyncDivSelector.setVisible(synced);
    };
    addAndMakeVisible(preDelayTempoSyncButton);

    // Freeze and bypass buttons
    freezeButton.setButtonText("FREEZE");
    addAndMakeVisible(freezeButton);

    bypassButton.setButtonText("BYPASS");
    addAndMakeVisible(bypassButton);

    // Meters
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    addAndMakeVisible(gateMeter);
    addAndMakeVisible(decayVisualizer);

    // Create APVTS attachments
    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "reverbType", typeSelector);
    algoModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "algoMode", algoModeSelector);
    shimmerPitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "shimmerPitch", shimmerPitchSelector);
    preDelaySyncDivAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "preDelaySyncDiv", preDelaySyncDivSelector);

    preDelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "preDelay", preDelaySlider);
    preDelayTempoSyncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "preDelayTempoSync", preDelayTempoSyncButton);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "decay", decaySlider);
    dampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "damping", dampingSlider);
    sizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "size", sizeSlider);
    diffusionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "diffusion", diffusionSlider);
    modRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "modRate", modRateSlider);
    modDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "modDepth", modDepthSlider);
    earlyLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "earlyLevel", earlyLevelSlider);
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "width", widthSlider);
    highPassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "highPass", highPassSlider);
    lowPassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "lowPass", lowPassSlider);
    duckingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "ducking", duckingSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "mix", mixSlider);

    shimmerAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "shimmerAmount", shimmerAmountSlider);
    shimmerInfiniteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "shimmerInfinite", shimmerInfiniteButton);

    springTensionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "springTension", springTensionSlider);
    springDripAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "springDrip", springDripSlider);
    springMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "springMix", springMixSlider);

    gateThresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "gateThreshold", gateThresholdSlider);
    gateHoldAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "gateHold", gateHoldSlider);
    gateReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "gateRelease", gateReleaseSlider);
    gateShapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "gateShape", gateShapeSlider);

    freezeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "freeze", freezeButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getAPVTS(), "bypass", bypassButton);

    // Initialize visibility
    updateVisibleControls();

    // Start timer for metering
    startTimerHz(30);

    setSize(800, 500);
}

DynoverbAudioProcessorEditor::~DynoverbAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void DynoverbAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    slider.setColour(juce::Slider::textBoxTextColourId, Colors::textPrimary);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, Colors::textSecondary);
    label.setFont(juce::Font(11.0f));
    addAndMakeVisible(label);
}

void DynoverbAudioProcessorEditor::setupComboBox(juce::ComboBox& comboBox, juce::Label& label, const juce::String& labelText, const juce::StringArray& items)
{
    comboBox.addItemList(items, 1);
    comboBox.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(comboBox);

    if (labelText.isNotEmpty())
    {
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, Colors::textSecondary);
        label.setFont(juce::Font(11.0f));
        addAndMakeVisible(label);
    }
}

void DynoverbAudioProcessorEditor::updateVisibleControls()
{
    ReverbType type = static_cast<ReverbType>(typeSelector.getSelectedId() - 1);

    // Hide all type-specific controls
    algoModeSelector.setVisible(false);
    algoModeLabel.setVisible(false);

    shimmerPitchSelector.setVisible(false);
    shimmerPitchLabel.setVisible(false);
    shimmerAmountSlider.setVisible(false);
    shimmerAmountLabel.setVisible(false);
    shimmerInfiniteButton.setVisible(false);

    springTensionSlider.setVisible(false);
    springTensionLabel.setVisible(false);
    springDripSlider.setVisible(false);
    springDripLabel.setVisible(false);
    springMixSlider.setVisible(false);
    springMixLabel.setVisible(false);

    gateThresholdSlider.setVisible(false);
    gateThresholdLabel.setVisible(false);
    gateHoldSlider.setVisible(false);
    gateHoldLabel.setVisible(false);
    gateReleaseSlider.setVisible(false);
    gateReleaseLabel.setVisible(false);
    gateShapeSlider.setVisible(false);
    gateShapeLabel.setVisible(false);

    gateMeter.setVisible(false);

    // Show relevant controls
    switch (type)
    {
        case ReverbType::Algorithmic:
            algoModeSelector.setVisible(true);
            algoModeLabel.setVisible(true);
            break;

        case ReverbType::Shimmer:
            shimmerPitchSelector.setVisible(true);
            shimmerPitchLabel.setVisible(true);
            shimmerAmountSlider.setVisible(true);
            shimmerAmountLabel.setVisible(true);
            shimmerInfiniteButton.setVisible(true);
            break;

        case ReverbType::Spring:
            springTensionSlider.setVisible(true);
            springTensionLabel.setVisible(true);
            springDripSlider.setVisible(true);
            springDripLabel.setVisible(true);
            springMixSlider.setVisible(true);
            springMixLabel.setVisible(true);
            break;

        case ReverbType::Gated:
            gateThresholdSlider.setVisible(true);
            gateThresholdLabel.setVisible(true);
            gateHoldSlider.setVisible(true);
            gateHoldLabel.setVisible(true);
            gateReleaseSlider.setVisible(true);
            gateReleaseLabel.setVisible(true);
            gateShapeSlider.setVisible(true);
            gateShapeLabel.setVisible(true);
            gateMeter.setVisible(true);
            break;
    }

    repaint();
}

void DynoverbAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(Colors::background);

    // Header area
    g.setColour(Colors::panelBg);
    g.fillRoundedRectangle(10.0f, 10.0f, getWidth() - 20.0f, 50.0f, 6.0f);

    // Title
    g.setColour(Colors::accent);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("DYNOVERB", 20, 15, 200, 40, juce::Justification::centredLeft);

    // Subtitle
    g.setColour(Colors::textSecondary);
    g.setFont(juce::Font(12.0f));
    g.drawText("Dynamic Reverb", 20, 38, 200, 20, juce::Justification::centredLeft);

    // Main panel
    g.setColour(Colors::panelBg);
    g.fillRoundedRectangle(10.0f, 70.0f, getWidth() - 20.0f, 180.0f, 6.0f);
    g.setColour(Colors::panelBorder);
    g.drawRoundedRectangle(10.0f, 70.0f, getWidth() - 20.0f, 180.0f, 6.0f, 1.0f);

    // Type-specific panel
    g.setColour(Colors::panelBg);
    g.fillRoundedRectangle(10.0f, 260.0f, getWidth() - 20.0f, 100.0f, 6.0f);
    g.setColour(Colors::panelBorder);
    g.drawRoundedRectangle(10.0f, 260.0f, getWidth() - 20.0f, 100.0f, 6.0f, 1.0f);

    // Output panel
    g.setColour(Colors::panelBg);
    g.fillRoundedRectangle(10.0f, 370.0f, getWidth() - 20.0f, 120.0f, 6.0f);
    g.setColour(Colors::panelBorder);
    g.drawRoundedRectangle(10.0f, 370.0f, getWidth() - 20.0f, 120.0f, 6.0f, 1.0f);

    // Section labels
    g.setColour(Colors::textSecondary);
    g.setFont(juce::Font(10.0f));
    g.drawText("MAIN CONTROLS", 20, 75, 150, 15, juce::Justification::centredLeft);
    g.drawText("TYPE CONTROLS", 20, 265, 150, 15, juce::Justification::centredLeft);
    g.drawText("OUTPUT", 20, 375, 150, 15, juce::Justification::centredLeft);

    // Meter labels
    g.drawText("IN", 20, 385, 20, 15, juce::Justification::centred);
    g.drawText("OUT", 50, 385, 25, 15, juce::Justification::centred);
}

void DynoverbAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    // Header
    auto headerArea = bounds.removeFromTop(60);
    typeLabel.setBounds(headerArea.removeFromRight(100).reduced(5));
    typeSelector.setBounds(headerArea.removeFromRight(120).reduced(5, 15));

    bypassButton.setBounds(headerArea.removeFromRight(80).reduced(5, 15));
    freezeButton.setBounds(headerArea.removeFromRight(80).reduced(5, 15));

    bounds.removeFromTop(10);

    // Main controls panel
    auto mainPanel = bounds.removeFromTop(180).reduced(5, 15);
    int knobWidth = 70;
    int knobHeight = 85;
    int labelHeight = 15;

    // First row - main parameters
    auto row1 = mainPanel.removeFromTop(knobHeight + labelHeight);

    auto preDelayArea = row1.removeFromLeft(knobWidth + 40);
    preDelayLabel.setBounds(preDelayArea.removeFromTop(labelHeight));
    preDelaySlider.setBounds(preDelayArea.removeFromLeft(knobWidth));
    preDelaySyncDivSelector.setBounds(preDelayArea.removeFromLeft(knobWidth).reduced(0, 20));
    auto syncArea = preDelayArea.removeFromLeft(35);
    preDelayTempoSyncButton.setBounds(syncArea.reduced(2, 25));

    auto decayArea = row1.removeFromLeft(knobWidth);
    decayLabel.setBounds(decayArea.removeFromTop(labelHeight));
    decaySlider.setBounds(decayArea);

    auto dampingArea = row1.removeFromLeft(knobWidth);
    dampingLabel.setBounds(dampingArea.removeFromTop(labelHeight));
    dampingSlider.setBounds(dampingArea);

    auto sizeArea = row1.removeFromLeft(knobWidth);
    sizeLabel.setBounds(sizeArea.removeFromTop(labelHeight));
    sizeSlider.setBounds(sizeArea);

    auto diffusionArea = row1.removeFromLeft(knobWidth);
    diffusionLabel.setBounds(diffusionArea.removeFromTop(labelHeight));
    diffusionSlider.setBounds(diffusionArea);

    // Decay visualizer
    auto visualizerArea = row1.removeFromLeft(140);
    decayVisualizer.setBounds(visualizerArea.reduced(10));

    // Second row - modulation and width
    auto row2 = mainPanel.removeFromTop(knobHeight + labelHeight);

    auto modRateArea = row2.removeFromLeft(knobWidth);
    modRateLabel.setBounds(modRateArea.removeFromTop(labelHeight));
    modRateSlider.setBounds(modRateArea);

    auto modDepthArea = row2.removeFromLeft(knobWidth);
    modDepthLabel.setBounds(modDepthArea.removeFromTop(labelHeight));
    modDepthSlider.setBounds(modDepthArea);

    auto earlyArea = row2.removeFromLeft(knobWidth);
    earlyLevelLabel.setBounds(earlyArea.removeFromTop(labelHeight));
    earlyLevelSlider.setBounds(earlyArea);

    auto widthArea = row2.removeFromLeft(knobWidth);
    widthLabel.setBounds(widthArea.removeFromTop(labelHeight));
    widthSlider.setBounds(widthArea);

    auto highPassArea = row2.removeFromLeft(knobWidth);
    highPassLabel.setBounds(highPassArea.removeFromTop(labelHeight));
    highPassSlider.setBounds(highPassArea);

    auto lowPassArea = row2.removeFromLeft(knobWidth);
    lowPassLabel.setBounds(lowPassArea.removeFromTop(labelHeight));
    lowPassSlider.setBounds(lowPassArea);

    bounds.removeFromTop(10);

    // Type-specific panel
    auto typePanel = bounds.removeFromTop(100).reduced(5, 15);

    // Algorithmic controls
    algoModeLabel.setBounds(typePanel.getX(), typePanel.getY(), 80, labelHeight);
    algoModeSelector.setBounds(typePanel.getX(), typePanel.getY() + labelHeight, 100, 25);

    // Shimmer controls
    shimmerPitchLabel.setBounds(typePanel.getX(), typePanel.getY(), 80, labelHeight);
    shimmerPitchSelector.setBounds(typePanel.getX(), typePanel.getY() + labelHeight, 100, 25);

    auto shimmerKnobArea = typePanel.withX(typePanel.getX() + 110).withWidth(knobWidth);
    shimmerAmountLabel.setBounds(shimmerKnobArea.removeFromTop(labelHeight));
    shimmerAmountSlider.setBounds(shimmerKnobArea.removeFromTop(knobHeight));
    shimmerInfiniteButton.setBounds(typePanel.getX() + 190, typePanel.getY() + 20, 80, 25);

    // Spring controls
    auto springArea = typePanel;
    auto tensionArea = springArea.removeFromLeft(knobWidth);
    springTensionLabel.setBounds(tensionArea.removeFromTop(labelHeight));
    springTensionSlider.setBounds(tensionArea.removeFromTop(knobHeight));

    auto dripArea = springArea.removeFromLeft(knobWidth);
    springDripLabel.setBounds(dripArea.removeFromTop(labelHeight));
    springDripSlider.setBounds(dripArea.removeFromTop(knobHeight));

    auto sMixArea = springArea.removeFromLeft(knobWidth);
    springMixLabel.setBounds(sMixArea.removeFromTop(labelHeight));
    springMixSlider.setBounds(sMixArea.removeFromTop(knobHeight));

    // Gated controls
    auto gatedArea = typePanel;
    auto threshArea = gatedArea.removeFromLeft(knobWidth);
    gateThresholdLabel.setBounds(threshArea.removeFromTop(labelHeight));
    gateThresholdSlider.setBounds(threshArea.removeFromTop(knobHeight));

    auto holdArea = gatedArea.removeFromLeft(knobWidth);
    gateHoldLabel.setBounds(holdArea.removeFromTop(labelHeight));
    gateHoldSlider.setBounds(holdArea.removeFromTop(knobHeight));

    auto releaseArea = gatedArea.removeFromLeft(knobWidth);
    gateReleaseLabel.setBounds(releaseArea.removeFromTop(labelHeight));
    gateReleaseSlider.setBounds(releaseArea.removeFromTop(knobHeight));

    auto shapeArea = gatedArea.removeFromLeft(knobWidth);
    gateShapeLabel.setBounds(shapeArea.removeFromTop(labelHeight));
    gateShapeSlider.setBounds(shapeArea.removeFromTop(knobHeight));

    gateMeter.setBounds(gatedArea.removeFromLeft(150).reduced(10, 25));

    bounds.removeFromTop(10);

    // Output panel
    auto outputPanel = bounds.reduced(5, 15);

    // Meters on the left
    inputMeter.setBounds(outputPanel.getX() + 5, outputPanel.getY() + 15, 15, 70);
    outputMeter.setBounds(outputPanel.getX() + 30, outputPanel.getY() + 15, 15, 70);

    // Output controls
    auto duckingArea = outputPanel.withX(outputPanel.getX() + 70).withWidth(knobWidth);
    duckingLabel.setBounds(duckingArea.removeFromTop(labelHeight));
    duckingSlider.setBounds(duckingArea.removeFromTop(knobHeight));

    auto mixArea = outputPanel.withX(outputPanel.getX() + 150).withWidth(knobWidth);
    mixLabel.setBounds(mixArea.removeFromTop(labelHeight));
    mixSlider.setBounds(mixArea.removeFromTop(knobHeight));
}

void DynoverbAudioProcessorEditor::timerCallback()
{
    // Smooth level metering
    float targetIn = audioProcessor.getInputLevel();
    float targetOut = audioProcessor.getOutputLevel();

    smoothedInputLevel = smoothedInputLevel * 0.8f + targetIn * 0.2f;
    smoothedOutputLevel = smoothedOutputLevel * 0.8f + targetOut * 0.2f;

    // Faster decay when signal drops
    if (targetIn < smoothedInputLevel)
        smoothedInputLevel *= 0.92f;
    if (targetOut < smoothedOutputLevel)
        smoothedOutputLevel *= 0.92f;

    inputMeter.setLevel(smoothedInputLevel);
    outputMeter.setLevel(smoothedOutputLevel);

    // Gate meter for gated reverb
    if (audioProcessor.getCurrentReverbType() == ReverbType::Gated)
    {
        gateMeter.setGateLevel(audioProcessor.getGateLevel());
    }

    // Update decay visualizer
    auto* decayParam = audioProcessor.getAPVTS().getRawParameterValue("decay");
    auto* freezeParam = audioProcessor.getAPVTS().getRawParameterValue("freeze");
    if (decayParam && freezeParam)
    {
        decayVisualizer.setDecay(decayParam->load(), freezeParam->load() > 0.5f);
    }
}
