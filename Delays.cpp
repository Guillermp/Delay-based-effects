#include <iostream>

#include <vector>
#include "Effects_V2.h"
#include "AudioFile.h"


int main() {
    // 1) Load input wav
    AudioFile<float> audio;
    if (!audio.load("demo_guitar.wav")) {
        std::cerr << "Failed to load input.wav\n";
        return 1;
    }

    const int sampleRate   = audio.getSampleRate();
    const int numChannels  = audio.getNumChannels();
    const int numSamples   = audio.getNumSamplesPerChannel();

    std::cout << "Loaded input.wav"
              << " | fs: " << sampleRate
              << " | Channels: " << numChannels
              << " | Samples/ch: " << numSamples
              << "\n";

    // 2) Set delay parameters
    float delayMs   = 251.0f;
    float wetness   = 1.0f;
    float feedback  = 0.5f;

    std::size_t delay_samples = static_cast<std::size_t> (std::llround((delayMs / 1000.0f) * sampleRate));

    // 3) Create one processor per channel (VERY IMPORTANT)
    std::vector<Flanger> effect(numChannels);
    
    for (int ch = 0; ch < numChannels; ++ch) {
        effect[ch].enter_parameters_manually();
        auto& s = audio.samples[ch];
        for (int n = 0; n < numSamples; ++n) {
            s[n] = effect[ch].process(s[n], sampleRate);
        }
    }

    // 5) Save output
    // (AudioFile supports WAV/AIFF; this saves WAV.)
    if (!audio.save("output.wav", AudioFileFormat::Wave)) {
        std::cerr << "Failed to save output.wav\n";
        return 1;
    }

    std::cout << "Wrote output.wav\n";
    return 0;
}
