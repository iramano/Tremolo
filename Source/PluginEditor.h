#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class TremoloLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TremoloLookAndFeel();

    juce::Font getTitleFont(float size) const;
    juce::Font getControlFont(float size) const;

    void drawRotarySlider(juce::Graphics& g,
        int x, int y,
        int width, int height,
        float sliderPos,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider& slider) override;

private:
    juce::Typeface::Ptr titleTypeface;
    juce::Typeface::Ptr controlTypeface;
};

class TremoloAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TremoloAudioProcessorEditor(TremoloAudioProcessor&);
    ~TremoloAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    TremoloAudioProcessor& audioProcessor;

    TremoloLookAndFeel customLookAndFeel;

    juce::Slider rateSlider;
    juce::Slider waveSlider;
    juce::Slider depthSlider;

    juce::Label rateLabel;
    juce::Label waveLabel;
    juce::Label depthLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    std::unique_ptr<SliderAttachment> rateAttachment;
    std::unique_ptr<SliderAttachment> waveAttachment;
    std::unique_ptr<SliderAttachment> depthAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloAudioProcessorEditor)
};