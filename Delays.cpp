#include <iostream>

#include <vector>
#include "Effects.h"
#include "AudioFile.h"


int main() {
    // 1) Load input wav
    AudioFile<float> audio;
    if (!audio.load("PCcity.wav")) {
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

    if (delay_samples >= BUF_SIZE) {
        std::cerr << "Delay too large for BUF_SIZE.\n"
                  << "delay_samples = " << delay_samples
                  << ", BUF_SIZE = " << BUF_SIZE << "\n"
                  << "Increase BUF_SIZE (power of two) or reduce delayMs.\n";
        return 1;
    }

    // 3) Create one processor per channel (VERY IMPORTANT)
    std::vector<DelayIIRProcessor> processors(static_cast<std::size_t>(numChannels));
    std::vector<VibratoProcessor> vibrato_processor(static_cast<std::size_t>(numChannels));

    // 4) Process samples in-place
    for (int ch = 0; ch < numChannels; ++ch) {
        auto& samples = audio.samples[ch]; // vector<float>
        auto& samples_vibrato = audio.samples[ch]; // vector<float>
        for (int n = 0; n < numSamples; ++n) {
            samples[n] = processors[static_cast<std::size_t>(ch)].process(samples[n], delay_samples, wetness, feedback);
            samples_vibrato[n] = vibrato_processor[static_cast<std::size_t>(ch)].process(samples[n], sampleRate,40,5);
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
