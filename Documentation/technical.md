# Technical documentation
## General architecture

The project is separated into a few main modules:

TODO: data flow diagram here

- Note source: responsible for sourcing note events and forwarding them to other modules
- Voice scheduler: responsible for updating the voice table according to received note events
- Voice table: the central data structure, containing the state of all voices
- Synthesizer: responsible for generating the sound for all voices
- Gui: responsible for drawing the user interface and handling the user's interaction with it

## Note source

This module is the least clear-cut one, since there are two sources
implemented. One is the midi port input handler, which resides in
`midi.c`, and the other is the on-screen keyboard residing in
`gui/keyboard.c`, which also handles drawing the keyboard and handling
input.

The main means of communication is the queue `note_events` (declared
in `note_source.h`), where values of type `note_event` are inserted.
Structure `note_event` is defined as follows:

```c
typedef enum { NE_DOWN, NE_UP } note_event_type;

typedef struct {
    uint8_t pitch;
    uint8_t velocity;
    note_event_type type;
} note_event;
```

The meaning of each field is:

- `pitch`: The midi number of the note (A0 is 21, G9 is 127)
- `velocity`: The midi velocity of the event (1 is the softest, 127 is the loudest)
- `type`: `NE_DOWN` for a key press event, `NE_UP` for a key release event

### On-screen Keyboard

TODO

### MIDI port

The midi receiver uses the usart 6 interface. A simple receiver
circuit with an optoisolator is required, as per the midi
specification.

![MIDI receiver schematic](img/midi-receiver-schematic.svg)

Power and ground should be connected to pins marked respectively 5V
and GND in the arduino-compatible connector. The RX label has to be
connected to the rx pin of usart6 interface, which is pin marked D0
on the connector.

It is worth mentioning, that even though the microcontroller on the
board normally works in 3.3V logic, almost all its pins are labeled as
5V-tolerant in the specification. As such, there should be no problems
with using a 5V optoisolator.

The midi communication uses a standard uart protocol with baud rate of
31250 bits per second. The only required configuration was to enable
usart6 in asynchronous mode, set the correct baud rate and enable the
USART6 interrupt.

I couldn't get the HAL routines for interrupt-based uart read to work,
so i had to implement a workaround. I created a custom interrupt
handler routine `midiIRQ` that's called from `USART6_IRQHandler`
before the HAL built-in handler. This routine just sends the byte from
the Read Data Register (RDR) into the `midi_bytes` queue. To initiate
receive a bit in the usart control register has to be set as seen in
the function `receiveByte()` in `midi.c`. This line has been copied
from the HAL receive routine.

For MIDI message decoding a last command byte is kept in a global
variable. The message length is computed based on that byte. When
enough bytes arrive, the message is decoded and dispatched. Channel of
the messages is ignored.

Supported MIDI messages:

- Note off (#8x)
- Note on (#9x)
- Control Change (#Bx)

All other commands are ignored.

## Voice Table

TODO

## Voice Scheduler

TODO

## Synthesizer

TODO

### Oscillator

TODO

### Envelope

TODO

### Mixer

TODO

### Sound output

TODO

## GUI

TODO
