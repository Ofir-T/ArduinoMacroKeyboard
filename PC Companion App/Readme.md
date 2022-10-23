### ArduinoMacroKeyboard Companion App
The companion app for the [Arduino Macro Keyboard](https://github.com/Ofir-T/ArduinoMacroKeyboard)


## Features
- GUI!
- Dynamic Device selection
- Configure bindings without an IDE/ no programming

## To-Do:
- [] Key press higlighting. (maybe add a textbox to test changes in, and whenever that's in focus, highlight keys)
- [V] Re-scan com ports (button/interval?) {on every time set selector comes to focus}
- [/] Automatic device recognition (boards.txt) (somewhat happens with list_comports(keyword))
- ~~[] Save conf vs. try conf (eeprom vs. ram)?~~
- [V] Notifications: save successful/failed etc. (forward the arduino messages to a messagebox?)
- [V] Update serial communication for compatibility with arduino sketch 2.0.
- [ ] Pass debug message to log file.
- [ ] Save/revert changes button, same for when closing the app
- [ ] Hide SetSelector while no device is connected

## Known Issues

## Notes
- ~~The app will recognize your device only if you plug it in BEFORE starting the app.~~
- The attached executable is only compiled for Windows 10, but the source is available for you to compile on your own system. 
- Application GUI was only tested on my laptop wih 1920x1080 resolution, so weridness might happen on other resolutions/aspect ratios.
