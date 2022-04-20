### ArduinoMacroKeyboard - Description
A simple 3D printed macro keyboard with dial, based on the arduino platform.
Now with a PC companion app to edit the bindings on the fly!

# Background
This is just the story of how this project came to be. if you want to go straight to buisness, skip to the introduction bellow.

I was annoyed of having to remember tons of obscure keyboard shortcuts for every program/OS that I use, so I wanted a way to have my own shortcuts/macros, without compromising the functions of my "normal" keyboard, thus I created this programmable macro keyboard. (Also, many game anti-cheat software block macro scripts, such as AutoHotKey, and since these boards can emulate HID devices, the computer sees them as any other "real" keyboard, and shouldn't block my chat-only-totally-not-cheating-please-don't-ban-me macros. But that is totally NOT the reason I made this keyboard ;D)

## Introduction
My goal is to create a project accesible for as many people as possible i.e. highest rate of "want it" to "made it!" conversion. 

With that goal in mind, I had to make it:
-  As-short-as-possible parts-list/BOM. Less searching/aquiring of parts needed, means the likelihood of actually starting the project goes up, and less likely you are to find out midway that you are out of m3x16 bolts and need to wait for that snail-mail aliexpress package you ordered a month ago with the new pack of m3 bolts. Sheesh!
- Also, because I based it on 3d printing, it needs to be, if possible, printable in one go on most print beds out there, and design the objects to be easily printable. So simple designs with few to no supports.
- Clear instructions and intuitive assembly. the Fewer distinct steps, the better. This also means that snap-together is better than bolts and nuts. 
- Customization. I know that I designed it first of all for me, and it is perfect for my usecase, but other people might want to tweak it a bit for their needs. Need to be careful with this one, as usually customizablity->complexity, so there is a fine balance to keep.

It is based on Arduino and uses the Atmega m32u4's (Arduino Micro\Leonardo) ability to emulate HID devices.
It is possible to combine this with AHK for contextual actions.
The Arduino sketch is standalone, but the companion app makes it very easy to change bindings on the fly.

Also, on Thingiverse:  https://www.thingiverse.com/thing:4628023.

## Features
- should work with any Arduino/other boards based on the Atmega 32u4 microcontroller (Micro, Leonardo), and arduino bootloader.
- Switchable sets of bindings on encoder long-press. (can be re-assigned to any key)
- editable key bindings without re-programming. (until reset, but will be fixed in future updates)
- up to a total of ~250 different bindings/keys supported. including encoder and different sets. (e.g. [7x7keys +5 encoder]x4 sets)

## Bill Of Matrials
1x Arduino Micro/Leonardo or compatible board. <example_link>
1x USB cable
1x Rotary encoder. <example_link>
nx Push buttons/key switches. (up to you. example keyboard has 9)
3x M3x6 bolts. 2 of them can be up to 16mm.
~1m of electrical wire
Keypad parts: encoder cap, key caps (can be 3d printed, example in "3D Files")
Tools: soldering iron, solder

## Instructions
In instructions.pdf

## To-Do:

# General
- [V] Seperate encoder scan from pad scan
- [V] Fix tx rx leds
- [V] Create a rotation function for use with an IMU (e.g. mpu6050), so that so that key bindings appear the same in all orientations(wheel on left, right, top ,bottom).
- [] Persistent profiles (eeprom).
- [] Design snap-on lid, and board mount to replace the bolts.
- [] upload .step files

# Key remapping:
- ~~[X] maybe a good idea to set a maximum number of sets pre-compilation, to ensure enough heap space is reserved~~
> Even now, with many features added, only 20% of ram and 50% of flash are used. Theres a lot of wiggle room.
- [] enable the use of consumer keycodes as well.

# Testing mode:
- [V] latency test on the fly.
- []  key press higlighting with in app

# First setup:
- ~~[X] test to find switch-pin correlation (maybe make scanPad() return return button data instead of doing action)~~
> will be taken care of in the app
- ~~[X] change navigation from char to physical buttons (also check for first time setup).~~
> can't even remember what I meant when I wrote this :D

## Known Issues:

# Arduino:
- longrepeatedhold also triggers held (at least one time)
- seems like seial communications is constantly on when emulating hid.
- changing encoder bindings is only supported for single-click right now. may be improved in the future.
- orientation doesn't check for square key grid (nXn). so it might act weird for non square layouts.

# App:
- The app will recognize your device only if you plug it in BEFORE starting the app.

## Notes:
- separate main.cpp into a header and source?
- far future: bluetooth + battery?

## App notes:
- The attached executable is only compiled for Windows 10, but the source is available for you to compile on your own system. 
- The exectutables with multidevice in the name will allow you to choose between arduino COM devices. I haven't tested it with more than one device yet.
- also, the AMK will reset to it's default bindings upon it's reset, but will retain the edited profile as long as it is powered. (will be taken care of in the future)
- Application GUI was only tested on my laptop wih 1920x1080 resolution, so weridness might happen on other resolutions/aspect ratios.
