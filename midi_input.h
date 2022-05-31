#ifndef __MIDI_INPUT_H__
#define __MIDI_INPUT_H__


#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "project_config.h"

class MIDIInput
{
public:
    MIDIInput(uart_inst_t *uart) {
        _uart = uart;
        uart_init(_uart, 31250);
        gpio_set_function(PIN_MIDI_RX, GPIO_FUNC_UART);
    }

    void process();

    void setNoteOnCallback(void (*callback)(uint8_t, uint8_t, uint8_t));
    void setNoteOffCallback(void (*callback)(uint8_t, uint8_t, uint8_t));
    void setCCCallback(void (*callback)(uint8_t, uint8_t, uint8_t));

private:
    const char MIDICH = 1;

    char MIDIRunningStatus;
    char MIDINote;
    char MIDILevel;

    void (*MIDINoteOnCallback)(uint8_t, uint8_t, uint8_t);
    void (*MIDINoteOffCallback)(uint8_t, uint8_t, uint8_t);
    void (*MIDICCCallback)(uint8_t, uint8_t, uint8_t);

    uart_inst_t *_uart;

};

#endif // __MIDI_INPUT_H__
