
#include <cstddef>  
#include <cmath>    
#include <array>

// -------------- Circular Buffer -------------- //
template<typename T, std::size_t Size>
class CircularBuffer{
    static_assert((Size & (Size - 1)) == 0,
              "Size must be a power of two");

    public:
        T getElement(std::size_t delay) const noexcept {
            delay &= (Size - 1); // clamp
            std::size_t readIndex = (writeIndex - delay) & (Size - 1);
            return buffer[readIndex];
        
        }

        void push(T element) noexcept {
            buffer[writeIndex] = element;
            writeIndex += 1;
            writeIndex = writeIndex & (Size - 1);
        }

    private:
        std::array<T, Size> buffer{};
        std::size_t writeIndex = 0;

};

// ------------------ Delay processor (your DelayIIR but per-channel) ------------------
// Choose a power-of-two big enough for your max delay.
// Example: 2 seconds at 48kHz = 96000 samples -> next power of 2 is 131072.
constexpr std::size_t BUF_SIZE = 131072;
struct DelayIIRProcessor {
    CircularBuffer<float, BUF_SIZE> input_buffer;
    CircularBuffer<float, BUF_SIZE> output_buffer;

    float process(float input, std::size_t delay_samples, float wetness, float feedback) noexcept {
        float delayed_input  = input_buffer.getElement(delay_samples);
        float delayed_output = output_buffer.getElement(delay_samples);

        input_buffer.push(input);

        float output = (1.0f - wetness) * input + wetness * delayed_input + feedback * delayed_output;

        output_buffer.push(output);
        return output;
    }
};

// ---------------- Vibrato processor ------------------ //
constexpr std::size_t BUF_SIZE_vibrato = 1024;
struct VibratoProcessor {
private:
    CircularBuffer<float, BUF_SIZE_vibrato> input_buffer;

    float phase = 0.0f; // [0, 2π)
public:
    // depthSamples: modulation depth in samples (e.g. 5..50)
    // rateHz: LFO rate in Hz (e.g. 0.5..8)
    // baseDelaySamples: constant delay offset in samples (must be >= depthSamples + 1) (it's the average delay)
    float process(float input, int Fs, float depthSamples, float rateHz) noexcept {
        input_buffer.push(input);

        // advance LFO phase
        const float twoPi = 6.283185307179586f;
        phase += twoPi * rateHz / float(Fs);
        if (phase >= twoPi) phase -= twoPi;

        // time-varying delay
        float baseDelaySamples = depthSamples + 1;
        float delay = baseDelaySamples + depthSamples * std::sin(phase); // DC signal + AC signal

        // clamp to safe range for interpolation
        // need delay and delay+1 valid
        if (delay < 1.0f) delay = 1.0f;
        if (delay > float(BUF_SIZE_vibrato - 2)) delay = float(BUF_SIZE_vibrato - 2);

        int d0 = int(std::floor(delay));
        float frac = delay - float(d0);

        float y0 = input_buffer.getElement(std::size_t(d0));
        float y1 = input_buffer.getElement(std::size_t(d0 + 1));

        return (1.0f - frac) * y0 + frac * y1; // iterpolated value (to a aproximate the value at the fractional delay time index) (illusion of continuous waveform)
    }
};
