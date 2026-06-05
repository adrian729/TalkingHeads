# TalkingHeads

A multi-effect audio plugin for processing voice/dialogue tracks, built with [JUCE](https://juce.com). It takes a **mono input** and produces a **stereo output**, running the signal through a chain of EQ, multiband compression, stereo imaging, and phaser stages.

- **Formats:** VST3, AU (macOS), Standalone
- **Type:** Audio effect (no MIDI)
- **Version:** 1.0.0

## Signal chain

```
Mono in → Pre-gain → Multiband EQ → Multiband Compressor → Stereo Imager → Phaser → Dry/Wet Blend → Stereo out
```

### Multiband EQ
- High-pass and low-pass filters with selectable slope (12/24/36/48 dB/oct) and per-filter bypass
- 3 peak bands (defaults at 750 Hz, 2 kHz, 4 kHz), each with frequency, gain, Q, and bypass

### Multiband Compressor
- 3 bands (low/mid/high) split with Linkwitz-Riley crossovers
- Adjustable low-mid (default 400 Hz) and mid-high (default 2 kHz) crossover frequencies
- Per-band threshold, attack, release, ratio, mute, and bypass

### Stereo Imager
- Widens the mono source using Haas-effect based processing with three modes: plain Haas, Haas with mono cancellation, and Haas with mid/side processing
- Original/auxiliary signal gain, width, center, delay time (0–50 ms), and a crossover frequency to keep lows mono

### Phaser
- LFO rate and depth, center frequency, feedback, and wet/dry mix

### Global
- Master bypass, pre-gain, and dry/wet blend
- All ~57 parameters are smoothed and exposed to the host via `AudioProcessorValueTreeState`

## Building

JUCE is included as a git submodule, so clone recursively:

```sh
git clone --recurse-submodules <repo-url>
# or, if already cloned:
git submodule update --init --recursive
```

### Requirements
- CMake ≥ 3.22
- A C++17 compiler
- macOS: deployment target 11.0+

### Build

```sh
cmake -B build
cmake --build build
```

Built plugins are copied to the system plugin folders automatically (`COPY_PLUGIN_AFTER_BUILD`). The Standalone app ends up under `build/TalkingHeads_artefacts/`.

> The `TalkingHeads.jucer` file is a legacy Projucer project; CMake is the supported build path.

## Project structure

```
CMakeLists.txt          Build configuration (JUCE plugin targets)
JUCE/                   JUCE framework (git submodule)
Source/
  PluginProcessor.*     Main audio processor and effect chain
  PluginEditor.*        GUI editor (placeholder — no controls yet)
  PluginStateManager.*  Parameter layout and APVTS management
  ParameterObject.*     Parameter wrapper with smoothing
  parameterTypes.h      Parameter ID definitions
  MultiBandEQ.* / EQBand.*
  MultiBandCompressor.* / CompressorBand.*
  Imager.*
```

## Status

Work in progress:
- The GUI is a skeleton (blank editor window); all control is via host-exposed parameters
- Latency reporting for the EQ/compressor/imager stages is incomplete
- Known buffer-size issues in the Imager (see TODOs in source)

## License

This project is licensed under the [GNU AGPLv3](LICENSE), matching the open-source license of the [JUCE framework](https://juce.com) (JUCE 8 is dual-licensed under AGPLv3 and a commercial license; this project uses the AGPLv3 option).
