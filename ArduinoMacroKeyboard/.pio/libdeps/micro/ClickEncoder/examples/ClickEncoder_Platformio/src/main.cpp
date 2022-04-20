#include <Arduino.h>

#include <ClickEncoder.h>
#include <TimerOne.h>

// Port Pin IDs for Arduino
constexpr uint8_t PIN_ENCA = 4;
constexpr uint8_t PIN_ENCB = 5;
constexpr uint8_t PIN_BTN = 3;
constexpr uint8_t ENC_STEPSPERNOTCH = 4;
constexpr bool BTN_ACTIVESTATE = LOW;

constexpr uint16_t SERIAL_BAUDRATE = 9600;
constexpr uint16_t ENC_SERVICE_US = 1000; // 1ms
constexpr uint8_t PRINT_BASE = 10;

// interval in which encoder values are fetched and printed in this demo
constexpr uint16_t PRINTINTERVAL_MS = 100;

static ClickEncoder exampleClickEncoder{PIN_ENCA, PIN_ENCB, PIN_BTN, ENC_STEPSPERNOTCH, BTN_ACTIVESTATE};
static TimerOne timer1;

// --- forward-declared function prototypes:
// Prints out button state
void printClickEncoderButtonState();
// Prints turn information (turn status, direction, Acceleration value)
void printClickEncoderValue();
// Prints accumulated turn information
void printClickEncoderCount();
// Timer callback routine
void timer1_isr();

void setup()
{
    // Use the serial connection to print out encoder's behavior
    Serial.begin(SERIAL_BAUDRATE);
    // Setup and configure "full-blown" ClickEncoder
    exampleClickEncoder.setAccelerationEnabled(true);
    exampleClickEncoder.setDoubleClickEnabled(true);
    exampleClickEncoder.setLongPressRepeatEnabled(true);

    Serial.println("Hi! This is the PLatformIO ClickEncoder Example Program.");
    Serial.println("When connected correctly: turn right should increase the value.");

    // When ClickEncoder initialized, attach service routine
    timer1.attachInterrupt(timer1_isr);
    timer1.initialize(ENC_SERVICE_US);
}

void loop()
{
    // Simulate other tasks of MCU
    _delay_ms(PRINTINTERVAL_MS);

    // Gets ClickEncoder's values and prints to serial for demonstration.
    printClickEncoderButtonState();
    printClickEncoderValue();
    printClickEncoderCount();
}

void timer1_isr()
{
    // This is the Encoder's worker routine. It will physically read the hardware
    // and all most of the logic happens here. Recommended interval for this method is 1ms.
    exampleClickEncoder.service();
}

void printClickEncoderButtonState()
{
    switch (exampleClickEncoder.getButton())
    {
    case Button::Clicked:
        Serial.println("Button clicked");
        break;
    case Button::DoubleClicked:
        Serial.println("Button doubleClicked");
        break;
    case Button::Held:
        Serial.println("Button Held");
        break;
    case Button::LongPressRepeat:
        Serial.println("Button longPressRepeat");
        break;
    case Button::Released:
        Serial.println("Button released");
        break;
    default:
        // no output for "Open" or "Closed" to not spam the console.
        break;
    }
}

void printClickEncoderValue()
{
    int16_t value = exampleClickEncoder.getIncrement();
    if (value != 0)
    {
        Serial.print("Encoder value: ");
        Serial.print(value, PRINT_BASE);
        Serial.print(" ");
    }
}

void printClickEncoderCount()
{
    static int16_t lastValue{0};
    int16_t value = exampleClickEncoder.getAccumulate();
    if (value != lastValue)
    {
        Serial.print("Encoder count: ");
        Serial.println(value, PRINT_BASE);
    }
    lastValue = value;
}