### ArduinoMacroKeyboard - Description
A simple 3D printed macro keyboard with dial, based on the arduino platform.
Now with a PC companion app to edit the bindings on the fly!

# Background
I was annoyed of having to remember tons of obscure keyboard shortcuts for every program/OS that I use, so I wanted a way to have my own shortcuts/macros, without compromising the functions of my "normal" keyboard, thus I created this programmable macro keyboard. (Also, many game anti-cheat software block macro scripts, such as AutoHotKey, and since these boards can emulate HID devices, the computer sees them as any other "real" keyboard, and shouldn't block my chat-only-totally-not-cheating-please-don't-ban-me macros. But that is totally NOT the reason I made this keyboard ;D)

## Introduction
My goal is to create a project accesible for as many people as possible i.e. highest rate of "want it" to "made it!" conversion. 

With that goal in mind, I had to make it:
-  As-short-as-possible parts-list/BOM. Less searching/aquiring of parts needed means the likelihood of actually starting the project goes up, and less likely you are to find out midway that you are out of m3x16 bolts and need to wait for that snail-mail package you ordered a month ago with the new pack of m3 bolts. Sheesh!
- Also, because I based it on 3d printing, it needs to be, if possible, printable in one go on most print beds out there, and design the objects to be easily printable. So simple designs with few to no supports.
- Clear instructions and intuitive assembly. the Fewer distinct steps, the better. This also means that snap-together is better than bolts and nuts. 
- Customization. I know that I designed it first of all for me, and it is perfect for my usecase, but other people might want to tweak it a bit for their needs. Need to be careful with this one, as usually customizablity->complexity, so there is a fine balance to keep.

It is based on Arduino and uses the Atmega m32u4's (Arduino Micro\Leonardo) ability to emulate HID devices.
It is possible to combine this with AHK for contextual actions.
The Arduino sketch is standalone, but the companion app makes it very easy to change bindings on the fly.

Also, on Thingiverse:  https://www.thingiverse.com/thing:4628023.

## Features
- Should work with any Arduino/other boards based on the Atmega 32u4 microcontroller (Micro, Leonardo).
- Multiple sets supported.
- On the fly remapping.
- up to a total of ~250 different bindings/keys supported. including encoder and different sets. (e.g. [7x7keys +5 encoder]x4 sets)

## Bill Of Matrials
1x Arduino Micro/Leonardo or compatible board. <example_link>
1x USB cable.
1x Rotary encoder. <example_link>
nx Push buttons/key switches. (up to you. example keyboard has 9)
3x M3x6 bolts. 2 of them can be up to 16mm.
~1m of electrical wire.
Keypad parts: encoder cap, key caps (can be 3d printed, example in "3D Files").
Tools: soldering iron, solder.

## Instructions
In instructions.pdf

## To-Do

# General
- [V] Seperate encoder scan from pad scan
- [V] Fix tx rx leds
- [V] Create a rotation function for use with an IMU (e.g. mpu6050), so that so that key bindings appear the same in all orientations(wheel on left, right, top ,bottom).
- [V] Persistent profiles (eeprom).
- [V] Improve serial communication to non-blocking {in progress}.
- [ ] Memory Management. {in progress}
- [ ] Cleanup.
- [ ] Documentation.
- [ ] Macros (action sequences).


# Enclosure:
- [ ] Snap-on lid
- [ ] Improved board mount (no screws).
- [ ] Upload .step files.
- [ ] Make all hardware mount on the top, to accomodate different bottoms.
    - Hot swappable keys? single pcb?

# Key remapping:
- [V] Enable the use of consumer keycodes, strings as well. 
    - Array of pointers to actions to do on key press?

## Known Issues

# Arduino:
- encoder doesn't play nice with command arrays. (maybe my encoder broke?)
- longrepeatedhold also triggers held (at least one time). (maybe change the hold behaviour to trigger on it's release, and have log repeatedhold block that if triggered.)
- Remapping encoder commands not yet implemented.
- orientation doesn't check for square key grid (nXn). so it might act weird for non square layouts. (maybe just make a reorientation function that re-slices the lists)


# App:

## Notes
- memory is very limited on th arduino micro, some restrictions are needed.

## App notes
- The executable is only compiled for Windows 10, but the source is available for you to compile on your own system. 
- Application GUI was only tested on my laptop wih 1920x1080 resolution, so weridness might happen on other resolutions/aspect ratios.

## Brain Dump
- far future: bluetooth + battery?
- change COM device name?
- pull cstr directly from eeprom to save ram?