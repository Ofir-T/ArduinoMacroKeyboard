// ----------------------------------------------------------------------------
// Rotary Encoder Driver with Acceleration
// Supports Click, DoubleClick, Held, LongPressRepeat
//
// Refactored, logic upgraded and feature added (LongPressRepeat) by Schallbert 2021
// ----------------------------------------------------------------------------

#ifndef CLICKENCODER_H
#define CLICKENCODER_H

#ifdef UNIT_TEST
    #include "ArduinoFake.h"
#else
    #include "Arduino.h"
#endif

// ----------------------------------------------------------------------------
// Acceleration configuration (for 1ms calls to ::service())
//
constexpr uint8_t ENC_ACCEL_START = 150; // The smaller this value, the quicker you must turn to activate acceleration.
constexpr uint8_t ENC_ACCEL_SLOPE = 75; // the smaller this value, the stronger the acceleration will manipulate values.

// Button configuration (values for 1ms timer service calls)
//
constexpr uint8_t ENC_BUTTONINTERVAL = 20;            // check button every x ms, also debouce time
constexpr uint16_t ENC_DOUBLECLICKTIME = 400;         // second click within x ms
constexpr uint16_t ENC_LONGPRESSREPEATINTERVAL = 200; // reports repeating-held every x ms
constexpr uint16_t ENC_HOLDTIME = 1200;               // report held button after x ms
// ----------------------------------------------------------------------------

class Encoder
{
public:
    explicit Encoder(uint8_t A, uint8_t B, uint8_t stepsPerNotch = 4, bool active = LOW);
    ~Encoder() = default;
    Encoder(const Encoder &cpyEncoder) = delete;
    Encoder &operator=(const Encoder &srcEncoder) = delete;

    void service();
    int16_t getIncrement();
    int16_t getAccumulate();
    void setAccelerationEnabled(const bool a) { accelerationEnabled = a; };

private:
    uint8_t getBitCode();
    void handleEncoder();
    int8_t handleAcceleration(int8_t direction);

    const uint8_t pinA;
    const uint8_t pinB;
    const uint8_t stepsPerNotch;
    const bool pinActiveState;

    bool accelerationEnabled{false};
    volatile uint8_t lastEncoderRead{0};
    volatile int16_t encoderAccumulate{0};
    volatile int16_t lastEncoderAccumulate{0};
    volatile uint8_t lastMovedCount{ENC_ACCEL_START};
};

class Button
{
public:
    enum eButtonStates
    {
        Open = 0,
        Closed,
        Held,
        LongPressRepeat,
        Released,
        Clicked,
        DoubleClicked
    };

    explicit Button(uint8_t BTN, bool active = LOW);
    ~Button() = default;
    Button(const Button &cpyButton) = delete;
    Button &operator=(const Button &srcButton) = delete;

    void service();
    eButtonStates getButton();
    void setDoubleClickEnabled(const bool b) { doubleClickEnabled = b; };
    // LongPressRepeat will overlay "Held" state. Thus, "Held" shouldn't be user in user code then!
    void setLongPressRepeatEnabled(const bool b) { longPressRepeatEnabled = b; };

private:
    void handleButton();
    void handleButtonPressed();
    void handleButtonReleased();

    const uint8_t pinBTN;
    const bool pinActiveState;

    bool doubleClickEnabled{false};
    bool longPressRepeatEnabled{false};
    volatile eButtonStates buttonState{Open};
    uint8_t doubleClickTicks{0};
    uint16_t keyDownTicks{0};
    uint16_t lastGetButtonCount{ENC_BUTTONINTERVAL};
};

class ClickEncoder
{
public:
    explicit ClickEncoder(uint8_t A, uint8_t B, uint8_t BTN = -1,
                 uint8_t stepsPerNotch = 4, bool active = LOW);
    ~ClickEncoder();
    ClickEncoder(const ClickEncoder &cpyEncoder) = delete;
    ClickEncoder &operator=(const ClickEncoder &srcEncoder) = delete;

    void service();
    // returns notch changes after last poll
    int16_t getIncrement() { return enc->getIncrement(); };
    // returns overall notch count since startup.
    int16_t getAccumulate() { return enc->getAccumulate(); };
    Button::eButtonStates getButton() {  return btn->getButton(); };
    // If active, encoder will count overproportionally quickly if turned fast.
    void setAccelerationEnabled(const bool b) { enc->setAccelerationEnabled(b); };
    void setDoubleClickEnabled(const bool b) { btn->setDoubleClickEnabled(b); };
    // LongPressRepeat will overlay "Held" state. Thus, "Held" shouldn't be user in user code then!
    void setLongPressRepeatEnabled(const bool b) { btn->setLongPressRepeatEnabled(b); };

private:
    Encoder* enc{nullptr};
    Button* btn{nullptr};
};
#endif // CLICKENCODER_H