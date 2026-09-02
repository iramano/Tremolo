#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr int editorWidth = 460;
    constexpr int editorHeight = 300;

    const juce::Colour backgroundColour = juce::Colour::fromRGB(67, 159, 153);   // #439F99
    const juce::Colour panelLineColour = juce::Colour::fromRGB(34, 83, 80);
    const juce::Colour textColour = juce::Colour::fromRGB(248, 241, 225);

    const juce::Colour knobOuterColour = juce::Colour::fromRGB(231, 218, 186);
    const juce::Colour knobInnerColour = juce::Colour::fromRGB(221, 206, 172);
    const juce::Colour knobRidgeLight = juce::Colour::fromRGBA(255, 255, 255, 65);
    const juce::Colour knobRidgeDark = juce::Colour::fromRGBA(90, 75, 45, 70);
    const juce::Colour pointerColour = juce::Colour::fromRGB(45, 44, 40);

    void setupSlider(juce::Slider& slider, TremoloLookAndFeel& lookAndFeel)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 74, 22);
        slider.setLookAndFeel(&lookAndFeel);

        slider.setColour(juce::Slider::textBoxTextColourId, pointerColour);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGBA(255, 255, 255, 190));
        slider.setColour(juce::Slider::textBoxOutlineColourId, panelLineColour);
        slider.setColour(juce::Slider::textBoxHighlightColourId, juce::Colours::transparentBlack);
    }

    void setupLabel(juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, textColour);
        label.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    }
}

TremoloLookAndFeel::TremoloLookAndFeel()
{
    titleTypeface =
        juce::Typeface::createSystemTypefaceFor(
            BinaryData::Fraunces_72pt_SuperSoftSemiBold_ttf,
            BinaryData::Fraunces_72pt_SuperSoftSemiBold_ttfSize
        );

    controlTypeface =
        juce::Typeface::createSystemTypefaceFor(
            BinaryData::Fraunces_72ptLightItalic_ttf,
            BinaryData::Fraunces_72ptLightItalic_ttfSize
        );
}

juce::Font TremoloLookAndFeel::getTitleFont(float size) const
{
    return juce::Font(
        juce::FontOptions(titleTypeface)
        .withHeight(size)
    );
}

juce::Font TremoloLookAndFeel::getControlFont(float size) const
{
    return juce::Font(
        juce::FontOptions(controlTypeface)
        .withHeight(size)
    );
}

void TremoloLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y,
    int width, int height,
    float sliderPos,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    juce::ignoreUnused(slider);

    auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height)
        .reduced(12.0f, 8.0f);

    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto centre = bounds.getCentre();

    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // sombra
    g.setColour(juce::Colour::fromRGBA(0, 0, 0, 35));
    g.fillEllipse(bounds.translated(0.0f, 3.0f));

    // corpo principal do knob
    g.setColour(knobOuterColour);
    g.fillEllipse(bounds);

    // estrias externas
    const int ridgeCount = 40;
    for (int i = 0; i < ridgeCount; ++i)
    {
        auto ridgeAngle = juce::MathConstants<float>::twoPi * (float)i / (float)ridgeCount;

        auto dir = juce::Point<float>(std::cos(ridgeAngle), std::sin(ridgeAngle));

        auto p1 = centre + dir * (radius * 0.77f);
        auto p2 = centre + dir * (radius * 0.98f);

        g.setColour((i % 2 == 0) ? knobRidgeDark : knobRidgeLight);
        g.drawLine({ p1, p2 }, 1.2f);
    }

    // tampa interna
    auto inner = bounds.reduced(radius * 0.16f);
    g.setColour(knobInnerColour);
    g.fillEllipse(inner);

    // brilho suave
    juce::ColourGradient gloss(
        juce::Colour::fromRGBA(255, 255, 255, 120),
        inner.getCentreX() - inner.getWidth() * 0.20f,
        inner.getY() + inner.getHeight() * 0.12f,
        juce::Colour::fromRGBA(255, 255, 255, 0),
        inner.getCentreX(),
        inner.getCentreY(),
        true
    );

    g.setGradientFill(gloss);
    g.fillEllipse(inner);

    // contorno
    g.setColour(juce::Colour::fromRGBA(70, 60, 45, 80));
    g.drawEllipse(bounds, 1.2f);
    g.drawEllipse(inner, 1.0f);

    // ponteiro preto, estilo knob vintage
    juce::Path pointer;
    pointer.addRoundedRectangle(-1.5f, -radius * 0.72f, 3.0f, radius * 0.56f, 1.0f);

    g.setColour(pointerColour);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));

    // miolo
    g.setColour(juce::Colour::fromRGBA(70, 60, 45, 110));
    g.fillEllipse(centre.x - 3.5f, centre.y - 3.5f, 7.0f, 7.0f);
}

TremoloAudioProcessorEditor::TremoloAudioProcessorEditor(TremoloAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setupSlider(rateSlider, customLookAndFeel);
    setupSlider(waveSlider, customLookAndFeel);
    setupSlider(depthSlider, customLookAndFeel);

    addAndMakeVisible(rateSlider);
    addAndMakeVisible(waveSlider);
    addAndMakeVisible(depthSlider);

    setupLabel(rateLabel, "rate");
    setupLabel(waveLabel, "wave");
    setupLabel(depthLabel, "depth");

    rateLabel.setFont(customLookAndFeel.getControlFont(17.0f));
    waveLabel.setFont(customLookAndFeel.getControlFont(17.0f));
    depthLabel.setFont(customLookAndFeel.getControlFont(17.0f));

    addAndMakeVisible(rateLabel);
    addAndMakeVisible(waveLabel);
    addAndMakeVisible(depthLabel);

    rateAttachment = std::make_unique<SliderAttachment>(audioProcessor.parameters, "rate", rateSlider);
    waveAttachment = std::make_unique<SliderAttachment>(audioProcessor.parameters, "wave", waveSlider);
    depthAttachment = std::make_unique<SliderAttachment>(audioProcessor.parameters, "depth", depthSlider);

    setSize(editorWidth, editorHeight);
}

TremoloAudioProcessorEditor::~TremoloAudioProcessorEditor()
{
    rateSlider.setLookAndFeel(nullptr);
    waveSlider.setLookAndFeel(nullptr);
    depthSlider.setLookAndFeel(nullptr);
}

void TremoloAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(backgroundColour);

    auto area = getLocalBounds().toFloat().reduced(10.0f);

    g.setColour(juce::Colour::fromRGBA(255, 255, 255, 22));
    g.fillRoundedRectangle(area, 14.0f);

    g.setColour(panelLineColour);
    g.drawRoundedRectangle(area, 14.0f, 2.0f);

    g.setColour(textColour);
    g.setFont(customLookAndFeel.getTitleFont(34.0f));

    g.drawFittedText(
        "tremolo2 chefe",
        0, 10, getWidth(), 44,
        juce::Justification::centred,
        1
    );

    g.setFont(customLookAndFeel.getControlFont(9.5f));

    g.setColour(textColour.withAlpha(0.75f));

    g.drawFittedText(
        "inspired by the early tr-2 circuit",
        0, getHeight() - 26, getWidth(), 14,
        juce::Justification::centred,
        1
    );
}

void TremoloAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(28, 78);

    auto columnWidth = area.getWidth() / 3;

    auto rateArea = area.removeFromLeft(columnWidth);
    auto waveArea = area.removeFromLeft(columnWidth);
    auto depthArea = area;

    auto placeControl = [](juce::Rectangle<int> bounds, juce::Label& label, juce::Slider& slider)
        {
            label.setBounds(bounds.removeFromTop(22));
            slider.setBounds(bounds.reduced(6));
        };

    placeControl(rateArea, rateLabel, rateSlider);
    placeControl(waveArea, waveLabel, waveSlider);
    placeControl(depthArea, depthLabel, depthSlider);
}