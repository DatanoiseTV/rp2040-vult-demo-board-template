#include "midi_input_usb.h"

void MIDIInputUSB::setNoteOnCallback(void (*callback)(uint8_t, uint8_t, uint8_t))
{
    MIDINoteOnCallback = callback;
}

void MIDIInputUSB::setNoteOffCallback(void (*callback)(uint8_t, uint8_t, uint8_t))
{
    MIDINoteOffCallback = callback;
}

void MIDIInputUSB::setCCCallback(void (*callback)(uint8_t, uint8_t, uint8_t))
{
    MIDICCCallback = callback;
}

void MIDIInputUSB::process()
{
    uint8_t packet[4];
    while (tud_midi_available())
    {
        tud_midi_read(packet, 4);
        if (packet[1] == 0x80 || packet[1] == 0x90)
        {
            if (packet[1] == 0x80)
            {
                if (MIDINoteOffCallback != NULL)
                {
                    MIDINoteOffCallback(packet[2], packet[3], packet[1] & 0xF);
                }
            }
            else
            {
                if (MIDINoteOnCallback != NULL)
                {
                    MIDINoteOnCallback(packet[2], packet[3], packet[1] & 0xF);
                }
            }
        }
        else if (packet[1] == 0xB0)
        {
            if (MIDICCCallback != NULL)
            {
                MIDICCCallback(packet[2], packet[3], packet[1] & 0xF);
            }
        }
    }
}
