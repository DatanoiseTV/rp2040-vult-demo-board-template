#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/interp.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"

#define USE_AUDIO_I2S 1
#include "audio_i2s.h"

#define SAMPLES_PER_BUFFER 256 // Samples / channel
audio_buffer_pool_t *ap;

static const uint32_t PIN_DCDC_PSM_CTRL = 23;

#include "project_config.h"

audio_buffer_pool_t *init_audio()
{

    static audio_format_t audio_format = {
        .sample_freq = 44100,
        .pcm_format = AUDIO_PCM_FORMAT_S32,
        .channel_count = (audio_channel_t)2};

    static audio_buffer_format_t producer_format = {
        .format = &audio_format,
        .sample_stride = 8};

    audio_buffer_pool_t *producer_pool = audio_new_producer_pool(&producer_format, 3,
                                                                 SAMPLES_PER_BUFFER); // todo correct size
    bool __unused ok;
    const audio_format_t *output_format;
#if USE_AUDIO_I2S
    audio_i2s_config_t config = {
        .data_pin = PIN_I2S_DOUT,
        .clock_pin_base = PIN_I2S_BCK,
        .dma_channel = 0,
        .pio_sm = 0};

    output_format = audio_i2s_setup(&audio_format, &audio_format, &config);
    if (!output_format)
    {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    { // initial buffer data
        audio_buffer_t *buffer = take_audio_buffer(producer_pool, true);
        int32_t *samples = (int32_t *)buffer->buffer->bytes;
        for (uint i = 0; i < buffer->max_sample_count; i++)
        {
            samples[i * 2 + 0] = 0;
            samples[i * 2 + 1] = 0;
        }
        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(producer_pool, buffer);
    }
    audio_i2s_set_enabled(true);
#elif USE_AUDIO_PWM
    output_format = audio_pwm_setup(&audio_format, -1, &default_mono_channel_config);
    if (!output_format)
    {
        panic("PicoAudio: Unable to open audio device.\n");
    }
    ok = audio_pwm_default_connect(producer_pool, false);
    assert(ok);
    audio_pwm_set_enabled(true);
#elif USE_AUDIO_SPDIF
    output_format = audio_spdif_setup(&audio_format, &audio_spdif_default_config);
    if (!output_format)
    {
        panic("PicoAudio: Unable to open audio device.\n");
    }
    // ok = audio_spdif_connect(producer_pool);
    ok = audio_spdif_connect(producer_pool);
    assert(ok);
    audio_spdif_set_enabled(true);
#endif
    return producer_pool;
}

static inline uint32_t _millis(void)
{
    return to_ms_since_boot(get_absolute_time());
}


int main()
{
    stdio_init_all();

    if (!set_sys_clock_khz(270000, false))
        printf("system clock 270MHz failed\n");
    else
        printf("system clock now 270MHz\n");

    // Initialize all Trigger inputs
    for(int pin=PIN_TRIG_IN_0; pin <= PIN_TRIG_BTN; pin++){
        gpio_init(pin);
        gpio_pull_up(pin);
        gpio_set_dir(pin, 0);
    }

    // Initialize MIDI input
    uart_init(uart1, 38400);
    gpio_set_function(PIN_MIDI_RX, GPIO_FUNC_UART);

    // DCDC PSM control
    // 0: PFM mode (best efficiency)
    // 1: PWM mode (improved ripple)
    gpio_init(PIN_DCDC_PSM_CTRL);
    gpio_set_dir(PIN_DCDC_PSM_CTRL, GPIO_OUT);
    gpio_put(PIN_DCDC_PSM_CTRL, 1); // PWM mode for less Audio noise

    adc_init();

    for(int pin=PIN_CV_IN_0; pin <= PIN_POT_1; pin++){
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        adc_gpio_init(pin);
    }


    // Add your background UI processing or midi etc.
    while(true){
        ;;
    }
}

#ifdef __cplusplus
extern "C"
{
#endif

    void i2s_callback_func()
    {
        audio_buffer_t *buffer = take_audio_buffer(ap, false);
        if (buffer == NULL)
        {
            return;
        }
        int32_t *samples = (int32_t *)buffer->buffer->bytes;

        for (uint i = 0; i < buffer->max_sample_count; i++)
        {
            // do your audio processing here
            samples[i * 2 + 0] = 0; // LEFT
            samples[i * 2 + 1] = 0; // RIGHT
        }
        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(ap, buffer);
        return;
    }

#ifdef __cplusplus
}
#endif