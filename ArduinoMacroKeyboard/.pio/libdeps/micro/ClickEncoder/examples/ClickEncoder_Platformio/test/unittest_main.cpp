#include <ArduinoFake.h>
#include <unity.h>

#include "unittest_main.h"

using namespace fakeit;

void setUp(void)
{
    ArduinoFakeReset();
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    // Button class unit tests
    RUN_TEST(button_constructor_activeLow_SetsInputPullup);
    RUN_TEST(button_constructor_activeHigh_Input);
    RUN_TEST(button_notPressed_Open);
    RUN_TEST(button_pressed_Closed);
    RUN_TEST(button_pressed_release_Clicked);
    RUN_TEST(button_click_getTwice_Open);
    RUN_TEST(button_pressedBelowThreshold_Closed);
    RUN_TEST(button_heldAboveThreshold_Held);
    RUN_TEST(button_heldAboveThreshold_release_Released);
    RUN_TEST(button_heldUntilLongPressRepeat_LongPressRepeat);
    RUN_TEST(button_heldUntilLongPressRepeat_keepHeld_Held);
    RUN_TEST(button_doubleclickWithinTime_doubleClicked);
    RUN_TEST(button_doubleclickNotWithinTime_Clicked);
    RUN_TEST(button_longPressRepeatOff_heldUntilLongPressRepeat_Held);
    RUN_TEST(button_doubleClickOff_doubleclick_Clicked);

    // Encoder class unit tests
    RUN_TEST(encoder_constructor_activeLow_setsInputPullup);
    RUN_TEST(encoder_constructor_activeHigh_setsInput);
    RUN_TEST(encoder_init_getIncrement0_getAccumulate0);
    RUN_TEST(encoder_noTurn_getIncrement0);
    RUN_TEST(encoder_turn1StepClockwise_getIncrement);
    RUN_TEST(encoder_turn3StepClockwise_getIncrement3_getAccumulate3);
    RUN_TEST(encoder_turn4StepClockwise_getIncrement4);
    RUN_TEST(encoder_turn5StepCounterClockwise_getIncrement_getAccumulate_equal);
    RUN_TEST(encoder_2stepBack2StepForth_getIncrement0);
    RUN_TEST(encoder_simulateJump0to2_getDecrement2);
    RUN_TEST(encoder_simulateJump1to3_getDecrement2);
    RUN_TEST(encoder_acceleration_initMove_wontAccelerate);
    RUN_TEST(encoder_acceleration_quickTurn);
    RUN_TEST(encoder_acceleration_slowTurn);
    RUN_TEST(encoder_moreStepsPerNotch_countsCorrectly);
    RUN_TEST(encoder_moreStepsPerNotch_acceleratesCorrectly);

    UNITY_END();
    return 0;
}