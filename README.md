# tremolo2 chefe

A VST3 tremolo plugin built with JUCE, inspired by the behavior and circuit topology of an early BOSS TR-2 revision using the M5207L01 VCA.

> Independent educational/research project. Not affiliated with, endorsed by, sponsored by, or associated with BOSS or Roland.

## About

`tremolo2 chefe` is a behavioral-modeling tremolo plugin.

The goal is to approximate documented TR-2 behavior using schematics, service documentation, component datasheets, waveform references, and measurements — without adding arbitrary "analog warmth" or vintage coloration that is not technically justified.

## Current Features

- Rate, Wave and Depth controls
- Approx. 1.11–11.11 Hz LFO range
- Triangle-to-square waveform behavior
- Asymmetric duty-cycle approximation
- M5207L01-inspired VCA model
- Control-path filtering
- Modeled audio coupling response
- Parameter smoothing and DAW automation
- Project state saving/restoration
- VST3 support

Currently developed and tested on Windows using FL Studio.

## Validation

The DSP has been tested using generated signals and compared with documented TR-2 behavior.

Measured examples:

```text
1 kHz   → effectively unity gain
100 Hz  → approx. -1.4 dB
50 Hz   → approx. -3.5 dB
The intermediate behavior of the WAVE control remains an approximation and may be refined.

Built With
- C++
- JUCE
- VST3
- Visual Studio

Disclaimer
BOSS, TR-2, Roland, and related trademarks belong to their respective owners.
tremolo2 chefe is an independent implementation created for educational, research, and audio-DSP development purposes.
This repository does not contain original BOSS/Roland source code, firmware, artwork, logos, or proprietary DSP code.
No claim is made that this software is an exact reproduction of any particular physical TR-2 unit.