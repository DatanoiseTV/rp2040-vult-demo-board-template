#include "midi_input.h"



void MIDIInput::setNoteOnCallback(void (*callback)(uint8_t, uint8_t, uint8_t))
{
    MIDINoteOnCallback = callback;
}

void MIDIInput::setNoteOffCallback(void (*callback)(uint8_t, uint8_t, uint8_t))
{
    MIDINoteOffCallback = callback;
}

void MIDIInput::setCCCallback(void (*callback)(uint8_t, uint8_t, uint8_t))
{
    MIDICCCallback = callback;
}

void MIDIInput::process()
{
    uint8_t mb = uart_getc (_uart);

    if ((mb >= 0x80) && (mb <= 0xEF))
    {
        // MIDI Voice Category Message.
        // Action: Start handling Running Status
        MIDIRunningStatus = mb;
        MIDINote = 0;
        MIDILevel = 0;
    }
    else if ((mb >= 0xF0) && (mb <= 0xF7))
    {

        // MIDI System Common Category Message.
        // Action: Reset Running Status.
        MIDIRunningStatus = 0;
    }
    else if ((mb >= 0xF8) && (mb <= 0xFF))
    {
        // System Real-Time Message.
        // Action: Ignore these.
        return;
    }
    else
    {
        // MIDI Data
        if (MIDIRunningStatus == 0)
            // No record of what state we're in, so can go no further
            return;

        if (MIDIRunningStatus >> 4 == (0x8))
        {
            uint8_t channel = MIDIRunningStatus & 0x0F;

            // Note OFF Received
            if (MIDINote == 0)
            {
                // Store the note number
                MIDINote = mb;
            }
            else
            {
                // Already have the note, so store the level
                MIDILevel = mb;
                if (MIDINoteOffCallback)
                    MIDINoteOffCallback(MIDINote, MIDILevel, channel);
                MIDINote = 0;
                MIDILevel = 0;
            }
        }
        else if (MIDIRunningStatus >> 4 == (0x9))
        {
            uint8_t channel = MIDIRunningStatus & 0x0F;

            // Note ON Received
            if (MIDINote == 0)
            {
                // Store the note number
                MIDINote = mb;
            }
            else
            {
                // Already have the note, so store the level
                MIDILevel = mb;
                if (MIDILevel == 0)
                {
                    if (MIDINoteOffCallback)
                        MIDINoteOffCallback(MIDINote, MIDILevel, channel);
                }
                else
                {
                    if (MIDINoteOnCallback)
                        MIDINoteOnCallback(MIDINote, MIDILevel, channel);
                    MIDINote = 0;
                    MIDILevel = 0;
                }
            }
        }
        else if (MIDIRunningStatus >> 4 == (0xB))
        {

            uint8_t channel = MIDIRunningStatus & 0x0F;

            // Control Change Received
            if (MIDINote == 0)
            {
                // Store the control number
                MIDINote = mb;
            }
            else
            {
                // Already have the control, so store the level
                MIDILevel = mb;
                if (MIDICCCallback)
                    MIDICCCallback(MIDINote, MIDILevel, channel);
                MIDINote = 0;
                MIDILevel = 0;
            }
        }
        else
        {
            // This is a MIDI command we aren't handling right now
            return;
        }
    }
}
