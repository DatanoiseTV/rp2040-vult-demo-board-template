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
#include "hardware/vreg.h"
#include "project_config.h"
#include "bsp/board.h"
#include "tusb.h"
#include "midi_input.h"
#include "midi_input_usb.h"
#include "audio_subsystem.h"
#include "vult.h"

#define USE_USB_MIDI 1

audio_buffer_pool_t *ap;

Dsp_process_type ctx;

#ifdef USE_DIN_MIDI
MIDIInput midi_input(uart1);
#elif defined(USE_USB_MIDI)
MIDIInputUSB midi_input;
#endif

typedef struct
{
    uint16_t ch0, ch1, ch2, ch3;
    bool gate1, gate2, gate3, gate4;
} cv_channels_t;

cv_channels_t cv;

static inline uint32_t _millis(void)
{
    return to_ms_since_boot(get_absolute_time());
}

void process_cv(void)
{
    // The rp2040 has multiplexed adc inputs.
    // TODO: This is very ugly.

    adc_select_input(0);
    cv.ch0 = adc_read();
    adc_select_input(1);
    cv.ch1 = adc_read();

    cv.gate1 = !gpio_get(PIN_TRIG_IN_0);
    cv.gate2 = !gpio_get(PIN_TRIG_IN_1);
    cv.gate3 = !gpio_get(PIN_TRIG_IN_2);
    cv.gate4 = !gpio_get(PIN_TRIG_IN_3);
}

// MIDI callbacks
void note_on_callback(uint8_t note, uint8_t level, uint8_t channel)
{
    Dsp_noteOn(ctx, note, level, channel);
    //printf("note on (ch %d): %d %d\n", channel, note, level);
    // gpio_put(24, 1);
}

void note_off_callback(uint8_t note, uint8_t level, uint8_t channel)
{
    Dsp_noteOff(ctx, note, channel);
    //printf("note off (ch %d): %d %d\n", channel, note, level);
    // gpio_put(24, 0);
}

void cc_callback(uint8_t cc, uint8_t value, uint8_t channel)
{
    Dsp_controlChange(ctx, cc, value, channel);
    //printf("cc (ch %d): %d %d\n", channel, cc, value);
}


#ifdef __cplusplus
extern "C"
{
#endif

int main()
{
    /*if (!set_sys_clock_khz(270000, false))
        printf("system clock 270MHz failed\n");
    else
        printf("system clock now 270MHz\n");
        */

    vreg_set_voltage(VREG_VOLTAGE_1_30);
    sleep_ms(1);
    set_sys_clock_khz(420000, true);


    #define DEBUGOMATIC_BOARD 1

    // Debugomatic has output buffers on each output which need
    // to be enabled first before I/O works!
    #ifdef DEBUGOMATIC_BOARD
        gpio_init(24);
        gpio_set_dir(24, 1);
        gpio_put(24, 1);

        gpio_init(26);
        gpio_set_dir(26, 1);
        gpio_put(26, 1);
    #endif

    stdio_init_all();

    board_init();
    tusb_init();

    // Initialize Vult DSP. This must match the DSP code.
    Dsp_process_init(ctx);
    Dsp_default_init(ctx);
    Dsp_default(ctx);


    #ifdef DINI
    // Initialize all Trigger inputs
    for (int pin = PIN_TRIG_IN_0; pin <= PIN_TRIG_BTN; pin++)
    {
        gpio_init(pin);
        gpio_pull_up(pin);
        gpio_set_dir(pin, 0);
    }

    adc_init();

    for (int pin = PIN_CV_IN_0; pin <= PIN_POT_1; pin++)
    {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        adc_gpio_init(pin);
    }
    #endif

    midi_input.setCCCallback(cc_callback);
    midi_input.setNoteOnCallback(note_on_callback);
    midi_input.setNoteOffCallback(note_off_callback);

 srand(1231293128);        
    ap = init_audio();
    // Add your background UI processing or midi etc.
    while (true)
    {
        tud_task();
        midi_input.process();
    }
}

    void i2s_callback_func()
    {
        //printf("i2s callback\n");
        audio_buffer_t *buffer = take_audio_buffer(ap, false);
        if (buffer == NULL)
        {
            printf("empty buffer\n");
            return;
        }
        int32_t *samples = (int32_t *)buffer->buffer->bytes;

        // process CV each round of samples
        //process_cv();

        // seed rng
       

        for (uint i = 0; i < buffer->max_sample_count; i++)
        {
            int32_t smp = Dsp_process(ctx, cv.ch0, cv.ch1, cv.gate1, cv.gate2, cv.gate3, cv.gate4) << 16u;
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