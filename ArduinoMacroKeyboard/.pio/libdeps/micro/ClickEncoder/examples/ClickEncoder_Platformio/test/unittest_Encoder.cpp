#include <ArduinoFake.h>
#include <ClickEncoder.h>

#include <unity.h>

using namespace fakeit;

uint8_t pinA{5};
uint8_t pinB{6};
uint8_t stepsPerNotch{1};
bool pinActiveState{false};
Encoder *encoder{nullptr};

void encoder_setup()
{
    When(Method(ArduinoFake(), pinMode)).AlwaysReturn();

    encoder = new Encoder{pinA, pinB, stepsPerNotch, pinActiveState};
    encoder->setAccelerationEnabled(false);
}

void encoder_teardown()
{
    delete encoder;
    encoder = nullptr;
}

void simulateEncoderService(uint16_t millisec)
{
    // simulate x ms elapse
    for (uint16_t i = 0; i < millisec; ++i)
    {
        encoder->service();
    }
}

void encoder_constructor_activeLow_setsInputPullup()
{
    When(Method(ArduinoFake(), pinMode)).AlwaysReturn();

    Encoder enc{pinA, pinB, stepsPerNotch, false};

    Verify(Method(ArduinoFake(), pinMode).Using(pinA, INPUT_PULLUP)).Once();
    Verify(Method(ArduinoFake(), pinMode).Using(pinB, INPUT_PULLUP)).Once();
}

void encoder_constructor_activeHigh_setsInput()
{
    When(Method(ArduinoFake(), pinMode)).AlwaysReturn();

    Encoder enc{pinA, pinB, stepsPerNotch, true};

    Verify(Method(ArduinoFake(), pinMode).Using(pinA, INPUT)).Once();
    Verify(Method(ArduinoFake(), pinMode).Using(pinB, INPUT)).Once();
}

// Encoder table
// GrayCode convert
//  A &&  B --> 0
//  A && !B --> 1
// !A && !B --> 2
// !A &&  B --> 3

void encoder_init_getIncrement0_getAccumulate0()
{
    encoder_setup();

    TEST_ASSERT_EQUAL(0, encoder->getIncrement());
    TEST_ASSERT_EQUAL(0, encoder->getAccumulate());
    encoder_teardown();
}

void encoder_noTurn_getIncrement0()
{
    encoder_setup();

    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    encoder->service();
    encoder->service();

    TEST_ASSERT_EQUAL(0, encoder->getIncrement());
    encoder_teardown();
}

void encoder_turn1StepClockwise_getIncrement()
{
    encoder_setup();

    // 0 --> 1 (1)
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(!pinActiveState);
    encoder->service();

    TEST_ASSERT_EQUAL(1, encoder->getIncrement());
    encoder_teardown();
}

void encoder_turn3StepClockwise_getIncrement3_getAccumulate3()
{
    encoder_setup();

    // 0 --> 3 (+3)
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(!pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(!pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    encoder->service();

    TEST_ASSERT_EQUAL(3, encoder->getIncrement());
    TEST_ASSERT_EQUAL(3, encoder->getAccumulate());
    encoder_teardown();
}

void encoder_turn4StepClockwise_getIncrement4()
{
    encoder_setup();

    // 0 --> 0 (+4)
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(!pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(!pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    encoder->service();

    TEST_ASSERT_EQUAL(4, encoder->getIncrement());
    encoder_teardown();
}

void encoder_turn5StepCounterClockwise_getIncrement_getAccumulate_equal()
{
    encoder_setup();
    int16_t incrementCount{0};

    // 3 <-- 0 (-5)
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(!pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(!pinActiveState);
    encoder->service();
    incrementCount += encoder->getIncrement();
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(!pinActiveState);
    encoder->service();
    incrementCount += encoder->getIncrement();

    TEST_ASSERT_EQUAL(incrementCount, encoder->getAccumulate());
    encoder_teardown();
}

void encoder_2stepBack2StepForth_getIncrement0()
{
    encoder_setup();

    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(!pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(!pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    encoder->service();

    TEST_ASSERT_EQUAL(0, encoder->getIncrement());
    encoder_teardown();
}

// This is bad as when the encoder "jumps a step", the system CANNOT detect turn direction anymore
// So we test vs a default behavior which is to return -2 in this case.
void encoder_simulateJump0to2_getDecrement2()
{
    encoder_setup();

    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(!pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(!pinActiveState);
    encoder->service();

    TEST_ASSERT_EQUAL(-2, encoder->getIncrement());
    encoder_teardown();
}

void encoder_simulateJump1to3_getDecrement2()
{
    encoder_setup();

    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(!pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    encoder->service();
    encoder->getIncrement(); // reset increment count as above is not "init state"
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(!pinActiveState);
    encoder->service();

    TEST_ASSERT_EQUAL(-2, encoder->getIncrement());
    encoder_teardown();
}

void encoder_acceleration_initMove_wontAccelerate()
{
    encoder_setup();
    encoder->setAccelerationEnabled(true);

    // system starts not on 0 as expected but on "1" which will be interpreted as move.
    // Requirement is to avoid this move being detected as accelerated move.
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(!pinActiveState);
    encoder->service();

    TEST_ASSERT_EQUAL(1, encoder->getIncrement());
    encoder_teardown();
}

void encoder_acceleration_quickTurn()
{
    encoder_setup();
    encoder->setAccelerationEnabled(true);

    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    encoder->service();
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(!pinActiveState);
    encoder->service();
    encoder->getIncrement(); // clear increment counter as acceleration is only allowed to be measured after first move
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(!pinActiveState);
    encoder->service();

    TEST_ASSERT_EQUAL((ENC_ACCEL_START / ENC_ACCEL_SLOPE + 1), encoder->getIncrement());
    encoder_teardown();
}

void encoder_acceleration_slowTurn()
{
    encoder_setup();
    encoder->setAccelerationEnabled(true);

    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    simulateEncoderService(ENC_ACCEL_START); // simulate long wait between pin changes
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(!pinActiveState);
    encoder->service();

    TEST_ASSERT_EQUAL(1, encoder->getIncrement());
    encoder_teardown();
}

void encoder_moreStepsPerNotch_countsCorrectly()
{
    When(Method(ArduinoFake(), pinMode)).AlwaysReturn();
    Encoder twoStep{pinA, pinB, 2, pinActiveState};
    twoStep.setAccelerationEnabled(false);

    // 5 steps back (2 full notches)
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    twoStep.service();
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(!pinActiveState);
    twoStep.service();
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(!pinActiveState);
    twoStep.service();
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    twoStep.service();
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    twoStep.service();
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(!pinActiveState);
    twoStep.service();

    TEST_ASSERT_EQUAL(-2, twoStep.getIncrement());
}

void encoder_moreStepsPerNotch_acceleratesCorrectly()
{
    When(Method(ArduinoFake(), pinMode)).AlwaysReturn();
    Encoder twoStep{pinA, pinB, 2, pinActiveState};
    twoStep.setAccelerationEnabled(true);

    // 5 steps back (2 full notches)
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    twoStep.service();
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(!pinActiveState);
    twoStep.service(); 
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(!pinActiveState);
    twoStep.service(); // acceleration will only take place starting below
    When(Method(ArduinoFake(), digitalRead).Using(pinA)).AlwaysReturn(pinActiveState);
    twoStep.service();
    When(Method(ArduinoFake(), digitalRead).Using(pinB)).AlwaysReturn(pinActiveState);
    twoStep.service(); // one notch with acceleration turned

    TEST_ASSERT_EQUAL(-(ENC_ACCEL_START / ENC_ACCEL_SLOPE + 1), twoStep.getIncrement());
}