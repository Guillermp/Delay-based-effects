# Digital Delay effects

## Introduction

A digital delay effect is implemented where the delay time (ms), wetness, and the feedback amount can be tweaked. 

The input audio file must be a .wav file. After running the main code, an output .wav file is produced after being processed.

The samples for the delay implementation are saved in a circular buffer. 

> Usage: Delays.cpp is the main file and Effects_V2 is the latest version of the modulation effects library

## Modulation Delay Effects

Vibrato, chorus, and flanger are **modulation effects based on a time-varying delay line**.
In all three, an LFO (Low Frequency Oscillator) modulates the delay.


Shared Delay Model

$D(n) = D_0 + A · sin(2πn\frac{f_{LFO}}{f_s})$

Where:
> - $D_0$: base delay or average delay (samples)
> - $A$: modulation depth (samples)
> - $f_{LFO}$: modulation rate (Hz)
> - $f_s$: sampling frequency (Hz)

which means that $\frac{f_{LFO}}{f_s}$ is the cycles of the LFO per sample.

All effects require:
- Fractional delay
- Interpolation between samples

---

### Vibrato

**Concept**
- Pure pitch modulation (no dry signal) consisting of a quasi-periodic variation in the pitch of the audio signal [1].
- In DSP this variation in pitch is achieved by **increasing and decreasing** the playback delay time periodically with the help of an LFO.
    - Amount of pitch shift depends on the rate of change of the delay and therefore on the two parameters:
        - A
        - $f_{LFO}$ 


**Characteristics**
- Dry signal: No
- Feedback: No
- Delay range: very small

**User parameters**
- Vibrato frequency (Hz): this determines the frequency of the LFO $f_{LFO}$
- Frequency variation $\Delta$ (%): maximum pitch deviation relative to the input frequency. This determines the maximum and minimum frequency ratio $f_{ratio}=1 \pm \Delta/100$


> **Note**
> - The vibrato frequency (Hz) sets the LFO frequency $f_{LFO}$.
> - The two parameters together determine the delay modulation depth $A$, according to: $$A = \frac{(f_{\text{ratio}} - 1)\, f_s}{2\pi f_{LFO}}$$
> where $f_s$ is the sampling frequency and $f_{\text{ratio}}$ is the desired pitch ratio.

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
- Interference between dry and delayed signal: the flanger effect is created by mixing a signal with a slightly shifted copy of itself, the destructive and constructive interference of the sigals creates a comb filter with notches equally spaced in the frequency spectrum. By modulating the dalay amount between the signals the comb filter is swept over time [1].
- In addition, the even space between the notches creates an audiable resonant pitch [1].
- The total delay is normally kept way below the echo perception (50ms-70ms). Normally around 1-10ms [1].
- The **delay model** parameters are found with the following formulas.
    >- $$D_0=(M_0+\frac{M_w}{2})$$
    >- $$A=\frac{M_w}{2}$$

**User Parameters**
- **Minimum delay $M_0$**: this is the *delay* knob. It sets the highest frequency the first notch will reach during the sweeping of the filter.
- **Sweep width $M_w$**: amplitude of the LFO. The maximum delay is $M_0+M_w$, which determines the lowest frequency reached by the first notch.

- **LFO rate**: usually 0.1-10Hz

#### Flanger – Parameter Guide

| Parameter | Typical Range | What it Controls | Audible Effect |
|---------|---------------|------------------|----------------|
| Minimum delay $M_0$ (ms) | 0.1 – 2.0 | Highest frequency reached by the first comb notch | Smaller → brighter, metallic • Larger → darker, hollow |
| Sweep width $M_w$ (ms) | 0.1 – 3.0 | Depth of the delay modulation (peak-to-peak) | Small → subtle motion • Large → dramatic “jet” sweep |
| LFO rate (Hz) | 0.05 – 2.0 | Speed of the sweep | Slow → smooth, evolving • Fast → intense, animated |
| Mix (wet/dry) | 0.3 – 0.7 | Strength of the comb filtering | 0.5 → deepest notches • Lower → more natural |
| Feedback (optional) | −0.7 – +0.7 | Amount of delayed signal fed back | + → metallic/jet • − → hollow/vocal |

**Characteristics**
- Dry signal: Yes
- Feedback: Maybe, you choose. The feedback sharpens the nodges of the comb filter creating a metallic sound. **Not implemented yet**.
- Delay range: very short


## References
[1] Reiss, J. D., & McPherson, A. P. (2026). Audio effects: theory, implementation and application. CRC Press.
