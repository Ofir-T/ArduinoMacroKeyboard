ClickEncoder
=============
![Title image](/img/Title.png)

> Arduino library to handle rotary encoders **with** buttons as a user input device.
Rotary Encoder with pushbutton Implementation [ClickEncoder]. Also provides `Button` only or `Encoder` only (without button), each as a class on its own.


- Timer-Based: Works on any IO-Pin.
- Supports rotary acceleration, so when the encoder is rotated more quickly, the encoders value will change over-proportionally.
- Button reports multiple states: `Open/Closed`, `Clicked`, `DoubleClicked`, `Held`, `Released`, and `LongPressRepeat`. 

Encoder and button can be connected to any input pin, as this library requires it's timer interrupt service routine ::service() to be called every millisecond. The example uses [TimerOne] for that.

See the example applications, optimized for Arduino IDE / PlatformIO IDE.
![Serial output of example code](/img/ExampleProgram.png)

### Encoder
The library supports **acceleration**, so when the encoder is rotated more quickly, the encoders' value will increment overproportionally fast.

Acceleration can be enabled or disabled at runtine using `setAccelerationEnabled(bool)` and tuned in a header constant.

For instance, it may make sense to enable acceleration for a long list to scroll through quickly, and switching it back off afterwards.

**Please note** parameters for acceleration, held, doubleClick, and longPressRepeat have been tuned for **1ms** intervals [TimerOne repo], and need to be changed if you decide to call the service method in another interval.

Depending on the type of your encoder, you can define use the constructors parameter `stepsPerNotch` an set it to either `1`, `2` or `4` steps per notch (most encoders I used have 4 steps per notch).

### Button
The Button reports multiple states: `Open/Closed`, `Clicked`, `DoubleClicked`, `Held`, `Released`, and `LongPressRepeat`. You can fine-tune the timings in the library's header file. 

If LongPressRepeat is configured, the button will repeatedly send a signal when it is held for a longer time. It is not recommended to evaluate both `Held` and `LongPressRepeat` at the same time as they are mutually exclusive.

`DoubleClick` and `LongPressRepeat` ability can be modified at runtime.

### Additional features
Algorithm has (new) complex glitch & contact bounce suppression that makes it very accurate. That's why the library takes a little more program space than other comparable libraries. 
Unittests have recently been added that verify button/encoder behavior. Written with PlatformIO using ArduinoFake and the Unity test framework:
![Unittests passing](/img/Unittest_Pass.png)


[TimerOne]:http://playground.arduino.cc/Code/Timer1
[TimerOne repo]:https://github.com/PaulStoffregen/TimerOne
[ClickEncoder]:https://github.com/Schallbert/encoder
