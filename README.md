# Digital Delay effects

## Introduction

A digital delay effect is implemented where the delay time (ms), wetness, and the feedback amount can be tweaked. 

The input audio file must be a .wav file. After running the main code, an output .wav file is produced after being processed.

The samples for the delay implementation are saved in a circular buffer. 

## Modulation Delay Effects

Vibrato, chorus, and flanger are **modulation effects based on a time-varying delay line**.
In all three, an LFO (Low Frequency Oscillator) modulates the delay.


Shared Delay Model

D(t) = D0 + A · sin(2π · fLFO · t)

Where:
- D0: base delay (samples)
- A: modulation depth (samples)
- fLFO: modulation rate (Hz)

All effects require:
- Fractional delay
- Interpolation between samples

---

### Vibrato

**Concept**
- Pure pitch modulation
- No dry signal


**Characteristics**
- Dry signal: No
- Feedback: No
- Delay range: very small

**Typical Parameters**
- Base delay: D0 ≈ A + 1
- Depth: 2–20 samples
- Rate: 4–7 Hz
- Mix: 100% wet

---

### Chorus

**Concept**
- Simulates multiple performers
- Mix of dry and modulated signal

**Characteristics**
- Dry signal: Yes
- Feedback: No (usually)
- Delay range: medium

**Typical Parameters**
- Base delay: 15–30 ms
- Depth: 1–5 ms
- Rate: 0.2–1.5 Hz
- Mix: 30–60%

---

### Flanger

**Concept**
- Moving comb filter
- Interference between dry and delayed signal


**Characteristics**
- Dry signal: Yes
- Feedback: Yes
- Delay range: very short

**Typical Parameters**
- Base delay: 0.5–3 ms
- Depth: 0.5–3 ms
- Rate: 0.05–2 Hz
- Mix: ~50%
- Feedback: 0.2–0.8



