#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TremoloAudioProcessorEditor::TremoloAudioProcessorEditor(TremoloAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    rateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    rateSlider.setTextValueSuffix(" Hz");
    addAndMakeVisible(rateSlider);
    rateSlider.textFromValueFunction = [](double value)
        {
            const double knob = value / 100.0;

            const double maxPeriod = 0.900;
            const double minPeriod = 0.090;

            const double period =
                maxPeriod + (minPeriod - maxPeriod) * knob;

            const double hz = 1.0 / period;

            return juce::String(hz, 2) + " Hz";
        };

    waveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    waveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    waveSlider.setTextValueSuffix(" %");
    addAndMakeVisible(waveSlider);

    depthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    depthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(depthSlider);

    depthSlider.setTextValueSuffix(" %");

    rateAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters,
            "rate",
            rateSlider
        );

    waveAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters,
            "wave",
            waveSlider
        );

    depthAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters,
            "depth",
            depthSlider
        );

    setSize(520, 300);
}

TremoloAudioProcessorEditor::~TremoloAudioProcessorEditor()
{
}

//==============================================================================
void TremoloAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(30, 30, 30));

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);

    g.drawFittedText(
        "TREMOLO",
        0, 20, getWidth(), 40,
        juce::Justification::centred,
        1
    );

    g.setFont(14.0f);

    g.drawFittedText(
        "RATE",
        70, 80, 100, 30,
        juce::Justification::centred,
        1
    );

    g.drawFittedText(
        "WAVE",
        210, 80, 100, 30,
        juce::Justification::centred,
        1
    );

    g.drawFittedText(
        "DEPTH",
        350, 80, 100, 30,
        juce::Justification::centred,
        1
    );
}

void TremoloAudioProcessorEditor::resized()
{
    rateSlider.setBounds(60, 110, 120, 120);
    waveSlider.setBounds(200, 110, 120, 120);
    depthSlider.setBounds(340, 110, 120, 120);
}