# ArduinoMacroKeyboard
A simple 3D printed macro keyboard with dial, based on the arduino platform.
Now with a PC companion app to edit the bindings on the fly!

To-Do:
    [V] seperate encoder scan from pad scan
    [V] fix tx rx leds
    [V] create rotating pad key mappings to accommodate different layouts (wheel on left, right, top ,bottom)
    [] combine keyboard, consumer actions into one vector
    [] make scanPad and scanEncoder sensitive to editor mode. (recieve the function of the action wanted?)

Key remapping:
    [V] maybe a good idea to set a maximum number of sets pre-compilation, to ensure enough heap space is reserved. flag the unused ones to skip (so Toggle() handles those).

Testing mode:
        [V] latency test on the fly.
        [] change default key set
        []  key testing


First setup:
        [X] test to find switch-pin correlation (maybe make scanPad() return return button data instead of doing action). - will be taken care of in the app
        [X] change navigation from char to physical buttons (also check for first time setup). - can't remember what I meant when I wrote this :D

Notes:
- longrepeatedhold also triggers held (at least one time)
- seems like seial communications is constantly on when emulating hid.
- changing encoder bindings is only supported for single-click right now. may be improved in the future.
- separate main.cpp into a header and source?

App notes:
- at the moment, the app will recognize your device only if you plug it in BEFORE starting the app.
- exectutables with multidevice in the name will allow you to choose between arduino COM devices. I havent tested it.
- also, the AMK will reset to it's default bindings upon device reset, but will retain the edited profile as long as it is powered. (will be taken care of in the future)
- application GUI was only tested on my laptop wih 1920x1080 resolution, so weridness might happen on other resolutions/aspect ratios.
