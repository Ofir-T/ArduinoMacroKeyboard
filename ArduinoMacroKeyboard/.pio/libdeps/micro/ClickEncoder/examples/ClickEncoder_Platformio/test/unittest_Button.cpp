

#include <ArduinoFake.h>
#include <ClickEncoder.h>

#include <unity.h>

using namespace fakeit;

uint8_t buttonPin{5};
bool buttonActiveState{false};
Button *button{nullptr};

void button_setup()
{
    When(Method(ArduinoFake(), pinMode)).Return();

    button = new Button{buttonPin, buttonActiveState};
    button->setDoubleClickEnabled(true);
    button->setLongPressRepeatEnabled(true);
}

void button_teardown()
{
    delete button;
    button = nullptr;
}

void simulateButtonService(uint16_t millisec)
{
    // simulate x ms elapse
    for (uint16_t i = 0; i < millisec; ++i)
    {
        button->service();
    }
}

void button_constructor_activeLow_SetsInputPullup()
{
    When(Method(ArduinoFake(), pinMode)).Return();

    Button btn{buttonPin, false};

    Verify(Method(ArduinoFake(), pinMode).Using(buttonPin, INPUT_PULLUP)).Once();
}

void button_constructor_activeHigh_Input()
{
    When(Method(ArduinoFake(), pinMode)).Return();

    Button btn{buttonPin, true};

    Verify(Method(ArduinoFake(), pinMode).Using(buttonPin, INPUT)).Once();
}

void button_notPressed_Open()
{
    button_setup();

    When(Method(ArduinoFake(), digitalRead)).Return(!buttonActiveState); // not pressed
    button->service();

    TEST_ASSERT_EQUAL(Button::Open, button->getButton());
    button_teardown();
}

void button_pressed_Closed()
{
    button_setup();

    When(Method(ArduinoFake(), digitalRead)).Return(buttonActiveState); // pressed
    button->service();

    TEST_ASSERT_EQUAL(Button::Closed, button->getButton());
    button_teardown();
}

void button_pressedBelowThreshold_Closed()
{
    button_setup();

    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed
    simulateButtonService(ENC_HOLDTIME - ENC_BUTTONINTERVAL);

    TEST_ASSERT_EQUAL(Button::Closed, button->getButton());
    button_teardown();
}
void button_heldAboveThreshold_Held()
{
    button_setup();

    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed
    simulateButtonService(ENC_HOLDTIME + 1);

    TEST_ASSERT_EQUAL(Button::Held, button->getButton());
    button_teardown();
}

void button_pressed_release_Clicked()
{
    button_setup();

    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed
    button->service();

    When(Method(ArduinoFake(), digitalRead)).Return(!buttonActiveState); // not pressed
    simulateButtonService(ENC_BUTTONINTERVAL);

    TEST_ASSERT_EQUAL(Button::Clicked, button->getButton());
    button_teardown();
}

void button_click_getTwice_Open()
{
    button_setup();

    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed
    button->service();

    When(Method(ArduinoFake(), digitalRead)).Return(!buttonActiveState); // not pressed
    simulateButtonService(ENC_BUTTONINTERVAL);
    button->getButton(); // returns clicked
    button->service();

    TEST_ASSERT_EQUAL(Button::Open, button->getButton());
    button_teardown();
}

void button_heldAboveThreshold_release_Released()
{
    button_setup();

    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed
    simulateButtonService(ENC_HOLDTIME + 1);

    When(Method(ArduinoFake(), digitalRead)).Return(!buttonActiveState); // not pressed
    simulateButtonService(ENC_BUTTONINTERVAL);

    TEST_ASSERT_EQUAL(Button::Released, button->getButton());
    button_teardown();
}

void button_heldUntilLongPressRepeat_LongPressRepeat()
{
    button_setup();

    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed
    simulateButtonService(ENC_HOLDTIME + ENC_LONGPRESSREPEATINTERVAL + 1);

    TEST_ASSERT_EQUAL(Button::LongPressRepeat, button->getButton());
    button_teardown();
}

void button_longPressRepeatOff_heldUntilLongPressRepeat_Held()
{
    button_setup();

    button->setLongPressRepeatEnabled(false);
    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed
    simulateButtonService(ENC_HOLDTIME + ENC_LONGPRESSREPEATINTERVAL + 1);

    TEST_ASSERT_EQUAL(Button::Held, button->getButton());
    button_teardown();
}

void button_heldUntilLongPressRepeat_keepHeld_Held()
{
    button_setup();
    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed
    simulateButtonService(ENC_HOLDTIME + ENC_LONGPRESSREPEATINTERVAL + 1);

    button->getButton();
    simulateButtonService(ENC_BUTTONINTERVAL);

    TEST_ASSERT_EQUAL(Button::Held, button->getButton());
    button_teardown();
}

void button_doubleclickWithinTime_doubleClicked()
{
    button_setup();

    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed
    button->service();
    
    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(!buttonActiveState); // not pressed
    simulateButtonService(ENC_BUTTONINTERVAL);
    
    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed again
    simulateButtonService(ENC_BUTTONINTERVAL);

    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(!buttonActiveState); // not pressed
    simulateButtonService(ENC_BUTTONINTERVAL);

    TEST_ASSERT_EQUAL(Button::DoubleClicked, button->getButton());
    button_teardown();
}

void button_doubleclickNotWithinTime_Clicked()
{
    button_setup();

    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed
    button->service();
    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(!buttonActiveState); // not pressed
    simulateButtonService(ENC_DOUBLECLICKTIME + 1); // wait too long

    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed again
    simulateButtonService(ENC_BUTTONINTERVAL);
    
    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(!buttonActiveState); // not pressed
    simulateButtonService(ENC_BUTTONINTERVAL);

    TEST_ASSERT_EQUAL(Button::Clicked, button->getButton());
    button_teardown();
}

void button_doubleClickOff_doubleclick_Clicked()
{
    button_setup();

    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed
    button->service();
    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(!buttonActiveState); // not pressed
    simulateButtonService(ENC_BUTTONINTERVAL); 

    button->setDoubleClickEnabled(false);

    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(buttonActiveState); // pressed again
    simulateButtonService(ENC_BUTTONINTERVAL);
    
    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(!buttonActiveState); // not pressed
    simulateButtonService(ENC_BUTTONINTERVAL);

    TEST_ASSERT_EQUAL(Button::Clicked, button->getButton());
    button_teardown();
}