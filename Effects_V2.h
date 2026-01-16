#include <iostream>
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
        // Checking if the buffer size is enough
        if (delay_samples >= BUF_SIZE) {
            std::cerr << "Delay too large for BUF_SIZE.\n"
                    << "delay_samples = " << delay_samples
                    << ", BUF_SIZE = " << BUF_SIZE << "\n"
                    << "Increase BUF_SIZE (power of two) or reduce delayMs.\n";
            return 1;
        }

        float delayed_input  = input_buffer.getElement(delay_samples);
        float delayed_output = output_buffer.getElement(delay_samples);

        input_buffer.push(input);

        float output = (1.0f - wetness) * input + wetness * delayed_input + feedback * delayed_output;

        output_buffer.push(output);
        return output;
    }
};



// ---------------- Modulation effect processor ------------------ //
constexpr std::size_t BUF_SIZE_mod = 2048;
struct ModulationFxProcessor {
private:
    CircularBuffer<float, BUF_SIZE_mod> input_buffer;

    float phase = 0.0f; // [0, 2π)
    bool first_entrance = true;

protected:
    struct Params {
        float rateHz = 0.5f;
        float depthSamples = 10.0f;
        float mix = 0.5f;
    } p;
    virtual float baseDelaySamples(int Fs) const noexcept = 0;
public:
    virtual void parameter_info() = 0;
    // depthSamples: modulation depth in samples (e.g. 5..50)
    // rateHz: LFO rate in Hz (e.g. 0.5..8)
    // baseDelaySamples: constant delay offset in samples (must be >= depthSamples + 1) (it's the average delay)
    float process(float input, int Fs, bool log=false) noexcept {
        input_buffer.push(input);

        // advance LFO phase
        const float twoPi = 6.283185307179586f;
        phase += twoPi * p.rateHz / float(Fs);
        if (phase >= twoPi) phase -= twoPi;

        // time-varying delay
        //float baseDelaySamples = depthSamples + 1;
        float bDelay = baseDelaySamples(Fs);
        float delay = bDelay + p.depthSamples * std::sin(phase); // DC signal + AC signal

        float max_delay = (bDelay + p.depthSamples)/float(Fs)*1000.0f;
        float min_delay = (bDelay - p.depthSamples)/float(Fs)*1000.0f;

        if (log && first_entrance){
            first_entrance = false;
              std::cout << "Modulation effect: Range of the delay" 
                        << " | max: (ms) " << max_delay
                        << " | min: (ms) " << min_delay
                        << "\n";
        }

        // clamp to safe range for interpolation
        // need delay and delay+1 valid
        if (delay < 1.0f) delay = 1.0f;
        if (delay > float(BUF_SIZE_mod - 2)) delay = float(BUF_SIZE_mod - 2);

        int d0 = int(std::floor(delay));
        float frac = delay - float(d0);

        float y0 = input_buffer.getElement(std::size_t(d0));
        float y1 = input_buffer.getElement(std::size_t(d0 + 1));

        float wet_signal = (1.0f - frac) * y0 + frac * y1;

        return p.mix*wet_signal + (1.0f-p.mix)*input; // iterpolated value (to a aproximate the value at the fractional delay time index) (illusion of continuous waveform)
    }
    // To manually set the parameters
    void set_rateHz(float r) { p.rateHz = r; }
    void set_depthSamples(float d) { p.depthSamples = d; }
    void set_mix(float m) { p.mix = m; }


    void enter_parameters_manually() {

        // Ask the user if he/she wants to enter the parameters manually 
        char choice;
        std::cout << "Use default parameters? (y/n): ";
        std::cin >> choice;

        if (choice == 'y' || choice == 'Y')
            return;


        parameter_info();
        float r, d, m;

        // Rate
        while (true) {
            std::cout << "Insert rate in Hz: ";
            if (std::cin >> r && std::isfinite(r)) {
                set_rateHz(r);
                std::cout << "Rate set to " << r << " Hz\n";
                break;
            }
            std::cout << "Invalid value. Please enter a valid number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        // Depth
        while (true) {
            std::cout << "Insert depth in samples: ";
            if (std::cin >> d && std::isfinite(d)) {
                set_depthSamples(d);
                std::cout << "Depth set to " << d << " samples\n";
                break;
            }
            std::cout << "Invalid value. Please enter a valid number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        // Mix
        while (true) {
            std::cout << "Insert wetness (0-1): ";
            if (std::cin >> m && std::isfinite(m)) {
                set_mix(m);
                std::cout << "Mix set to " << m << "\n";
                break;
            }
            std::cout << "Invalid value. Please enter a valid number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }




};

// ---------------- Vibrato -------------------------------------- //
class Vibrato : public ModulationFxProcessor {
    public:
        Vibrato() {
        std::cout << "Vibrato effect loaded \n";
        //Constructor with common values for the effect
        p.rateHz = 5.0f;
        p.depthSamples = 10.0f;
        p.mix = 1.0f;
        }
        void parameter_info() override {
            std::cerr
                << "Vibrato — typical parameter ranges\n"
                << "---------------------------------\n"
                << "Mix (wetness):      1.0 (100% wet)\n"
                << "Base delay:         depth + 1 samples (internal)\n"
                << "Depth:              2 - 20 samples\n"
                << "LFO rate:           4 - 7 Hz\n"
                << "\n";
        }
    protected:
    float baseDelaySamples(int Fs) const noexcept override {
        return p.depthSamples + 1.0f;
    }

};

// ---------------- Chorus -------------------------------------- //
class Chorus : public ModulationFxProcessor {
public:
    Chorus() {
        std::cout << "Chorus effect loaded \n";
        // Constructor with common values for the effect
        p.rateHz = 0.8f;        // slow modulation
        p.depthSamples = 96.0f; // ~2 ms, since 48 kHz
        p.mix = 0.5f;           // dry + wet
    }

    void parameter_info() override {
        std::cerr
            << "Chorus — typical parameter ranges\n"
            << "--------------------------------\n"
            << "Mix (wetness):      0.3 - 0.6\n"
            << "Base delay:         15 - 30 ms (internal)\n"
            << "Depth:              1 - 5 ms\n"
            << "LFO rate:           0.2 - 1.5 Hz\n"
            << "\n";
    }

protected:
    float baseDelaySamples(int Fs) const noexcept override {
        return 0.020f * Fs; // 20 ms
    }
};

// ---------------- Flanger -------------------------------------- //
class Flanger : public ModulationFxProcessor {
public:
    Flanger() {
        // Constructor with common values for the effect
        std::cout << "Flanger effect loaded \n";
        p.rateHz = 0.2f;         // slow sweep
        p.depthSamples = 48.0f;  // ~1 ms, since 48 kHz
        p.mix = 0.5f;
    }

    void parameter_info() override {
        std::cerr
            << "Flanger — typical parameter ranges\n"
            << "---------------------------------\n"
            << "Mix (wetness):      ~0.5\n"
            << "Base delay:         0.5 - 3 ms (internal)\n"
            << "Depth:              0.5 - 3 ms\n"
            << "LFO rate:           0.05 - 2 Hz\n"
            << "\n";
    }

protected:
    float baseDelaySamples(int Fs) const noexcept override {
        return 0.001f * Fs; // 1 ms
    }
};
