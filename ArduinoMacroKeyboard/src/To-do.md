# ArduinoMacroKeyboard
A simple 3D printed macro keyboard with dial, based on the arduino platform.

To-Do:
    [V] seperate encoder scan from pad scan
    [V] fix tx rx leds
    - create rotating pad key mappings to accommodate different layouts (wheel on left, right, top ,bottom)
    - change toggle related names to something that implies the command sets
    - combine keyboard, consumer actions into one vector
    [] make scanPad and scanEncoder sensitive to editor mode. (recieve the function of the action wanted?)

    Key remapping:
        -maybe a good idea to set a maximum number of sets pre-compilation, to ensure enough heap space is reserved. flag the unused ones to skip (so Toggle() handles those).

    Editor mode:
        [V] latency test on the fly.
        - change default key set
        - one big macro keyboard class?

    First setup:
        - test to find switch-pin correlation (maybe make scanPad() return return button data instead of doing action)
        - change navigation from char to physical buttons (also check for first time setup)

Notes:
    - longrepeatedhold also triggers held (at least one time)
    -seems like seial communications is constantly on when emulating hid.