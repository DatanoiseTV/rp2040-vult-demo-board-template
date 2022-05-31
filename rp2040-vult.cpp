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
#include "project_config.h"
#include "audio_subsystem.h"
#include "vult.h"

static const uint32_t PIN_DCDC_PSM_CTRL = 23;
audio_buffer_pool_t *ap;

Dsp_process_type ctx;

static inline uint32_t _millis(void)
{
    return to_ms_since_boot(get_absolute_time());
}


int main()
{
    stdio_init_all();

    Dsp_process_init(ctx);

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
            int32_t smp = Dsp_process(ctx, 10240, 10240, 0);
            samples[i * 2 + 0] = smp; // LEFT
            samples[i * 2 + 1] = smp; // RIGHT
        }
        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(ap, buffer);
        return;
    }

#ifdef __cplusplus
}
#endif