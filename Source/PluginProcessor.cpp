/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TremoloAudioProcessor::TremoloAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
#else
    : parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
}

TremoloAudioProcessor::~TremoloAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
TremoloAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "rate",
        "Rate",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        50.0f
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "wave",
        "Wave",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "depth",
        "Depth",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        100.0f
    ));

    return layout;
}

//==============================================================================
const juce::String TremoloAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TremoloAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TremoloAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TremoloAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TremoloAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TremoloAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TremoloAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TremoloAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TremoloAudioProcessor::getProgramName (int index)
{
    return {};
}

void TremoloAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TremoloAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    phase = 0.0f;
    controlFilterState = 1.0f;

    rateSmoother.reset(sampleRate, 0.05);
    waveSmoother.reset(sampleRate, 0.05);
    depthSmoother.reset(sampleRate, 0.05);

    rateSmoother.setCurrentAndTargetValue(
        parameters.getRawParameterValue("rate")->load() / 100.0f
    );

    waveSmoother.setCurrentAndTargetValue(
        parameters.getRawParameterValue("wave")->load() / 100.0f
    );

    depthSmoother.setCurrentAndTargetValue(
        parameters.getRawParameterValue("depth")->load() / 100.0f
    );

    auto audioHighPassCoefficients =
        juce::dsp::IIR::Coefficients<float>::makeHighPass(
            sampleRate,
            59.0f
        );

    audioHighPassLeft.coefficients =
        audioHighPassCoefficients;

    audioHighPassRight.coefficients =
        audioHighPassCoefficients;

    audioHighPassLeft.reset();
    audioHighPassRight.reset();
}

void TremoloAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TremoloAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

float TremoloAudioProcessor::tr2VcaGain(float controlDeltaVolts) const
{
    constexpr float maxControlVoltage = 1.0f;

    const float vc =
        juce::jlimit(0.0f,
            maxControlVoltage,
            controlDeltaVolts);

    // M5207L01:
    // Vc = 1 V -> aproximadamente 100% / 0 dB.
    // Controle aproximadamente linear em amplitude.
    const float normalizedGain =
        vc / maxControlVoltage;

    // Piso aproximado documentado: -100 dB.
    const float minimumGain =
        juce::Decibels::decibelsToGain(-100.0f);

    return minimumGain
        + normalizedGain * (1.0f - minimumGain);
}

float TremoloAudioProcessor::tr2Lfo(float phaseNormalized,
    float waveKnob) const
{
    const float knob =
        juce::jlimit(0.0f, 1.0f, waveKnob);

    constexpr float waveTaperExponent = 2.0f;

    const float wave =
        std::pow(knob, waveTaperExponent);

    // Mantém a fase dentro de 0..1.
    const float phase =
        phaseNormalized - std::floor(phaseNormalized);

    // Extremidade triangular.
    const float triangle =
        1.0f - 4.0f * std::abs(phase - 0.5f);

    // Aproximação inicial do duty cycle assimétrico do TR-2.
    // A documentação confirma duty ratio específico,
    // mas não publica o percentual exato.
    constexpr float tr2Duty = 0.67f;

    const float highStart =
        (1.0f - tr2Duty) * 0.5f;

    const float highEnd =
        highStart + tr2Duty;

    const float square =
        (phase >= highStart && phase < highEnd)
        ? 1.0f
        : -1.0f;

    // WAVE: triangle -> square.
    return triangle * (1.0f - wave)
        + square * wave;
}

void TremoloAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    const double sampleRate = getSampleRate();

    // Limpa canais de saída que não possuem entrada correspondente.
    for (auto i = totalNumInputChannels;
        i < totalNumOutputChannels;
        ++i)
    {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    // Define os novos valores-alvo dos parâmetros.
    rateSmoother.setTargetValue(
        parameters.getRawParameterValue("rate")->load() / 100.0f
    );

    waveSmoother.setTargetValue(
        parameters.getRawParameterValue("wave")->load() / 100.0f
    );

    depthSmoother.setTargetValue(
        parameters.getRawParameterValue("depth")->load() / 100.0f
    );

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        // Valores suavizados dos knobs.
        const float rateKnob = rateSmoother.getNextValue();
        const float waveKnob = waveSmoother.getNextValue();
        const float depth = depthSmoother.getNextValue();

        //==============================================================
        // RATE
        // TR-2 antigo: aproximadamente 900 ms até 90 ms por ciclo.
        const float maxPeriod = 0.900f;
        const float minPeriod = 0.090f;

        const float period =
            maxPeriod + (minPeriod - maxPeriod) * rateKnob;

        const float rate = 1.0f / period;

        //==============================================================

        const float phaseNormalized =
            phase / juce::MathConstants<float>::twoPi;

        const float lfo =
            tr2Lfo(phaseNormalized, waveKnob);

        //==============================================================
        // DEPTH + aproximação do controle do M5207L01

        const float lfoControl =
            (lfo + 1.0f) * 0.5f;

        const float controlDeltaVolts =
            1.0f - depth * (1.0f - lfoControl);

        // Aproximação da rede R8 (100k) + C4 (0.1uF)
        // no caminho de controle do TR-2.
        const float controlCutoff = 17.5f;

        const float controlAlpha =
            1.0f - std::exp(
                -juce::MathConstants<float>::twoPi
                * controlCutoff
                / static_cast<float>(sampleRate)
            );

        controlFilterState +=
            controlAlpha * (controlDeltaVolts - controlFilterState);

        const float filteredControlDelta =
            controlFilterState;

        const float vcaGain =
            tr2VcaGain(filteredControlDelta);

        //==============================================================
        // Processamento do áudio

        for (int channel = 0;
            channel < totalNumInputChannels;
            ++channel)
        {
            auto* channelData =
                buffer.getWritePointer(channel);

            float processedSample =
                channelData[sample] * vcaGain;

            if (channel == 0)
                processedSample =
                audioHighPassLeft.processSample(processedSample);
            else if (channel == 1)
                processedSample =
                audioHighPassRight.processSample(processedSample);

            channelData[sample] = processedSample;
        }

        //==============================================================
        // Avança a fase do LFO

        phase += juce::MathConstants<float>::twoPi
            * rate
            / static_cast<float>(sampleRate);

        if (phase >= juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;
    }
}

//==============================================================================
bool TremoloAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TremoloAudioProcessor::createEditor()
{
    return new TremoloAudioProcessorEditor (*this);
}

//==============================================================================
void TremoloAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    copyXmlToBinary(*xml, destData);
}

void TremoloAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(
        getXmlFromBinary(data, sizeInBytes)
    );

    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(
                juce::ValueTree::fromXml(*xmlState)
            );
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TremoloAudioProcessor();
}
