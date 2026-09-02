# Tremolo

A VST3 tremolo plugin built with JUCE, inspired by the behavior and circuit topology of an early BOSS TR-2 revision using the M5207L01 VCA.

> This is an independent educational/research project and is not affiliated with, endorsed by, or sponsored by BOSS or Roland.

## About

The goal of this project is to build a tremolo effect that approximates the documented behavior of an early TR-2 circuit as closely as possible, while avoiding arbitrary "analog" coloration that is not supported by the available schematic, service documentation, datasheets, or measurements.

The current model includes:

- Rate control based on the documented TR-2 LFO range
- Wave control transitioning from triangle to square
- Asymmetric LFO duty cycle approximation
- Depth control
- M5207L01 VCA-inspired gain control
- Control-signal smoothing based on the circuit
- Audio coupling high-pass behavior derived from the schematic
- Parameter smoothing
- DAW parameter state saving/restoration
- Stereo-compatible processing using the same modulation on both channels

## Current status

The core DSP is functional and has been tested in FL Studio.

Some behaviors have been compared against the original TR-2 service notes, including:

- LFO minimum and maximum rate
- Triangle and square waveform behavior
- Depth response
- Output waveform behavior using a 200 Hz square-wave test
- Low-frequency response of the audio path

The intermediate behavior of the WAVE control is still an approximation and may be refined in future versions.

## Controls

### Rate

Controls the tremolo speed.

The current model uses an approximate range of:

- ~1.11 Hz minimum
- ~11.11 Hz maximum

### Wave

Morphs the LFO from a smooth triangular waveform toward a more square/choppy waveform.

### Depth

Controls the amount of amplitude modulation.

At minimum depth, the tremolo modulation is effectively disabled.

## Technical approach

The project uses behavioral circuit modeling rather than a full component-by-component electrical simulation.

The implementation is based on:

- TR-2 schematic analysis
- TR-2 service documentation
- M5207L01 VCA documentation
- measured/tested behavior inside the DAW

The project intentionally avoids adding saturation, noise, drift, compression, or other "vintage" effects unless there is technical evidence that they are relevant to the original circuit.

## Built with

- C++
- JUCE
- Visual Studio
- VST3

## Development environment

Currently developed and tested on Windows with FL Studio.

## Repository structure

```text
Source/
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h

Tremolo.jucer