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
#define EFFECT_DURATION_MS 5000
#define SAMPLES_PER_EFFECT ((SAMPLE_RATE * EFFECT_DURATION_MS) / 1000)

int16_t sine_wave[BUFFER_SIZE];
int16_t square_wave[BUFFER_SIZE];
int16_t saw_wave[BUFFER_SIZE];

void generate_sine_wave() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        float t = (float)i / BUFFER_SIZE;
        sine_wave[i] = (int16_t)(32767.0f * sinf(2 * M_PI * t));
    }
}

void generate_square_wave() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        square_wave[i] = (i < BUFFER_SIZE / 2) ? 32767 : -32768;
    }
}

void generate_saw_wave() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        saw_wave[i] = (int16_t)(65535 * ((float)i / BUFFER_SIZE) - 32768);
    }
}

int main() {
    stdio_init_all();
    generate_sine_wave();
    generate_square_wave();
    generate_saw_wave();

    PIO pio = pio0;
    uint sm = 0;

    uint offset = pio_add_program(pio, &i2s_program);
    i2s_program_init(pio, sm, offset, AUDIO_PIN_DATA, AUDIO_PIN_BCLK, AUDIO_PIN_LRCK);

    int sample_index = 0;
    int effect = 0; // 0 = sine, 1 = square, 2 = saw
    uint32_t effect_sample_counter = 0;

    while (true) {
        while (pio_sm_is_tx_fifo_full(pio, sm))
            tight_loop_contents();

        int16_t sample;
        if (effect == 0)
            sample = sine_wave[sample_index];
        else if (effect == 1)
            sample = square_wave[sample_index];
        else
            sample = saw_wave[sample_index];

        uint32_t stereo_sample = ((uint32_t)sample << 16) | (sample & 0xFFFF);
        pio_sm_put_blocking(pio, sm, stereo_sample);

        sample_index = (sample_index + 1) % BUFFER_SIZE;
        effect_sample_counter++;

        if (effect_sample_counter >= SAMPLES_PER_EFFECT) {
            effect = (effect + 1) % 3;
            effect_sample_counter = 0;
        }
    }
}
