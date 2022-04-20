// ----------------------------------------------------------------------------
// Rotary Encoder Driver with Acceleration
// Supports Click, DoubleClick, Held, LongPressRepeat
//
// Refactored, logic upgraded and feature added (LongPressRepeat) by Schallbert 2021
// ----------------------------------------------------------------------------

#include "ClickEncoder.h"

// ----------------------------------------------------------------------------

// Encoders typically have 3 pins: A, B, C (GND)
// Most of them have notches and register 4 steps (ticks) per notch.
// If mixed up A and B, encoder will turn "backwards".
Encoder::Encoder(uint8_t A,
                 uint8_t B,
                 uint8_t stepsPerNotch,
                 bool active) : pinA(A),
                                pinB(B),
                                stepsPerNotch(stepsPerNotch),
                                pinActiveState(active)
{
    uint8_t configType = (pinActiveState == LOW) ? INPUT_PULLUP : INPUT;
    pinMode(pinA, configType);
    pinMode(pinB, configType);
}

// Button pin BTN and active state to be defined.
Button::Button(uint8_t BTN,
               bool active) : pinBTN(BTN),
                              pinActiveState(active)
{
    uint8_t configType = (pinActiveState == LOW) ? INPUT_PULLUP : INPUT;
    pinMode(pinBTN, configType);
}

/// ClickEncoders typically have 5 pins: A, B, C (enc GND), BTN, GND
ClickEncoder::ClickEncoder(
    uint8_t A,
    uint8_t B,
    uint8_t BTN,
    uint8_t stepsPerNotch,
    bool active)
{
    enc = new Encoder(A, B, stepsPerNotch, active);
    btn = new Button(BTN, active);
}

ClickEncoder::~ClickEncoder()
{
    delete enc;
    delete btn;
    enc = nullptr;
    btn = nullptr;
}

// ----------------------------------------------------------------------------
// call this every 1 millisecond via timer ISR
void ClickEncoder::service(void)
{
    enc->service();
    btn->service();
}

// call this every 1 millisecond via timer ISR
void Encoder::service()
{
    handleEncoder();
}

// call this every 1 millisecond via timer ISR
void Button::service()
{
    ++lastGetButtonCount;
    handleButton();
}

// ----------------------------------------------------------------------------

void Encoder::handleEncoder()
{
    uint8_t encoderRead = getBitCode();
    // bit0 set = status changed, bit1 set = "overflow 3" where it goes 0->3 or 3->0
    uint8_t rawMovement = encoderRead - lastEncoderRead;
    lastEncoderRead = encoderRead;
    // This is the uint->int magic, converts raw to: -1 counterclockwise, 0 no turn, 1 clockwise
    int8_t signedMovement = ((rawMovement & 1) - (rawMovement & 2));

    encoderAccumulate += signedMovement;
    encoderAccumulate += handleAcceleration(signedMovement);
}

uint8_t Encoder::getBitCode()
{
    // GrayCode convert
    // !A && !B --> 0
    // !A &&  B --> 1
    //  A &&  B --> 2
    //  A && !B --> 3
    uint8_t currentEncoderRead = digitalRead(pinA);
    currentEncoderRead |= (currentEncoderRead << 1);
    
    // invert result's 0th bit if set
    currentEncoderRead ^= digitalRead(pinB);
    return currentEncoderRead;
}

int8_t Encoder::handleAcceleration(int8_t direction)
{
    if (lastMovedCount < ENC_ACCEL_START)
    {
        ++lastMovedCount;
    }

    if (direction == 0 || !accelerationEnabled || (encoderAccumulate % stepsPerNotch))
    {
        return 0;
    }

    // only when moved, only with enabled acceleration, only accelerate for "full notches", no movements in between
    int16_t acceleration = ((ENC_ACCEL_START / ENC_ACCEL_SLOPE) - (lastMovedCount / ENC_ACCEL_SLOPE));
    lastMovedCount = 0;
    if (direction > 0)
    {
        return acceleration;
    }
    else
    {
        return -acceleration;
    }
}
// ----------------------------------------------------------------------------

// returns number of notches that the encoder was turned since the last poll
// takes acceleration into account if configured
int16_t Encoder::getIncrement()
{
    int16_t accu = getAccumulate();
    int16_t encoderIncrements = accu - lastEncoderAccumulate;
    lastEncoderAccumulate = accu;
    return (encoderIncrements);
}

// returns sum of notches that the encoder was turned since startup
// takes acceleration into account if configured
int16_t Encoder::getAccumulate()
{
    return (encoderAccumulate / stepsPerNotch);
}

// ----------------------------------------------------------------------------
void Button::handleButton()
{
    if (lastGetButtonCount < ENC_BUTTONINTERVAL)
    {
        return;
    }
    lastGetButtonCount = 0;

    if (digitalRead(pinBTN) == pinActiveState)
    {
        handleButtonPressed();
    }
    else
    {
        handleButtonReleased();
    }

    if (doubleClickTicks > 0)
    {
        --doubleClickTicks;
    }
}

void Button::handleButtonPressed()
{
    buttonState = Closed;
    ++keyDownTicks;
    if (keyDownTicks >= (ENC_HOLDTIME / ENC_BUTTONINTERVAL))
    {
        buttonState = Held;
        if (!longPressRepeatEnabled)
        {
            return;
        }

        // Blip out LongPressRepeat once per interval
        if (keyDownTicks > ((ENC_LONGPRESSREPEATINTERVAL + ENC_HOLDTIME) / ENC_BUTTONINTERVAL))
        {
            buttonState = LongPressRepeat;
        }
    }
}

void Button::handleButtonReleased()
{
    keyDownTicks = 0;
    if (buttonState == Held)
    {
        buttonState = Released;
    }
    else if (buttonState == Closed)
    {
        buttonState = Clicked;
        if (!doubleClickEnabled)
        {
            return;
        }

        if (doubleClickTicks == 0)
        {
            // reset counter and wait for another click
            doubleClickTicks = (ENC_DOUBLECLICKTIME / ENC_BUTTONINTERVAL);
        }
        else
        {
            //doubleclick active and not elapsed!
            buttonState = DoubleClicked;
            doubleClickTicks = 0;
        }
    }
}

Button::eButtonStates Button::getButton(void)
{
    volatile Button::eButtonStates result{buttonState};
    if (result == LongPressRepeat)
    {
        // Reset to "Held"
        keyDownTicks = (ENC_HOLDTIME / ENC_BUTTONINTERVAL);
    }

    // reset after readout. Conditional to neither miss nor repeat DoubleClicks or Helds
    if (buttonState != Closed)
    {
        buttonState = Open;
    }

    return result;
}