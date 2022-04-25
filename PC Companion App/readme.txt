### ArduinoMacroKeyboard Companion App
The companion app for the [Arduino Macro Keyboard](https://github.com/Ofir-T/ArduinoMacroKeyboard)


## Features
- GUI!
- Device selection
- Configure bindings without an IDE/ no programming

## To-Do:
- [] key press higlighting
- [] re-scan com ports (button/interval?)
- [] Automatic device recognition (boards.txt)
- [] Save conf vs. try conf (eeprom vs. ram)
- [] notifications: save successful/failed etc. (forward the arduino messages to a messagebox?)

## Known Issues
- "Save Changes" button does nothing, conf is saved on combobox change

## Notes
- The app will recognize your device only if you plug it in BEFORE starting the app.
- The attached executable is only compiled for Windows 10, but the source is available for you to compile on your own system. 
- The exectutables with multidevice in the name will allow you to choose between arduino COM devices. I haven't tested it with more than one device yet.
- also, the AMK will reset to it's default bindings upon it's reset, but will retain the edited profile as long as it is powered. (will be taken care of in the future)
- Application GUI was only tested on my laptop wih 1920x1080 resolution, so weridness might happen on other resolutions/aspect ratios.
