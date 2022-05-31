#include "midi_input.h"


void MIDIInput::process()
{
    uint8_t mb = uart_getc (uart1);

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

        if (MIDIRunningStatus == (0x80 | (MIDICH - 1)))
        {
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
                    MIDINoteOffCallback(MIDINote, MIDILevel);
                MIDINote = 0;
                MIDILevel = 0;
            }
        }
        else if (MIDIRunningStatus == (0x90 | (MIDICH - 1)))
        {
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
                        MIDINoteOffCallback(MIDINote, MIDILevel);
                }
                else
                {
                    if (MIDINoteOnCallback)
                        MIDINoteOnCallback(MIDINote, MIDILevel);
                    MIDINote = 0;
                    MIDILevel = 0;
                }
            }
        }
        else if (MIDIRunningStatus == (0xB0 | (MIDICH - 1)))
        {
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
                    MIDICCCallback(MIDINote, MIDILevel);
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
