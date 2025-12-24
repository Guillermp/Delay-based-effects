#include <iostream>
#include <array>

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

CircularBuffer<float, 4> input_buffer;
CircularBuffer<float, 4> output_buffer;
float simpleDelay(float input){
    std::size_t delay_samples = 2;
    

    // Read delayed value
    float delayed_sample = input_buffer.getElement(delay_samples);

    // Insert new input to the buffer
    input_buffer.push(input);

    // Compute the output
    float M = 0; // Percentage of wet signal
    float output = (1 - M)*input + M*delayed_sample;

    return output;

}

float DelayIIR(float input, std::size_t delay_samples, float wetness, float feedback ){

    // Read delayed input and output values
    float delayed_input = input_buffer.getElement(delay_samples);
    float delayed_output = output_buffer.getElement(delay_samples);


    // Insert new input to the buffer
    input_buffer.push(input);

    // Compute the output

    float output = (1 - wetness)*input + wetness*delayed_input + feedback*delayed_output;

    // Store output in the output buffer
    output_buffer.push(output); 

    return output;

}

int main() {
    float signal[8] = {0.5, 0.25, 0.15, 0.0, -0.5, -0.25, -0.15, 0.0};
    float output;

    for (int i = 0; i < 8; i++) {
        output = DelayIIR(signal[i], 1, 1.0f, 0.2f);
        std::cout << output << std::endl;
        
    }

    




}