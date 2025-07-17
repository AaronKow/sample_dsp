#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "i2s.pio.h"
#include <math.h>
#include <stdio.h>

#define AUDIO_PIN_DATA 2    // I2S DIN pin to MAX98357
#define AUDIO_PIN_BCLK 3    // I2S BCLK pin to MAX98357
#define AUDIO_PIN_LRCK 4    // I2S LRC pin to MAX98357

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 256

int16_t sine_wave[BUFFER_SIZE];

void generate_sine_wave() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        float t = (float)i / BUFFER_SIZE;
        sine_wave[i] = (int16_t)(32767.0f * sinf(2 * M_PI * t));
    }
}

int main() {
    stdio_init_all();
    generate_sine_wave();

    PIO pio = pio0;
    uint sm = 0;

    uint offset = pio_add_program(pio, &i2s_program);
    i2s_program_init(pio, sm, offset, AUDIO_PIN_DATA, AUDIO_PIN_BCLK, AUDIO_PIN_LRCK);

    int sample_index = 0;
    while (true) {
        while (pio_sm_is_tx_fifo_full(pio, sm))
            tight_loop_contents();

        int16_t sample = sine_wave[sample_index];
        // 16 bits left + 16 bits right = 32 bits
        uint32_t stereo_sample = ((uint32_t)sample << 16) | (sample & 0xFFFF);
        pio_sm_put_blocking(pio, sm, stereo_sample);

        sample_index = (sample_index + 1) % BUFFER_SIZE;
    }
}
