#ifndef UNITTEST_BUTTON_H
#define UNITTEST_BUTTON_H

// all function prototypes of all tests shall be placed here

// BUTTON
void button_constructor_activeLow_SetsInputPullup();
void button_constructor_activeHigh_Input();
void button_notPressed_Open();
void button_pressed_Closed();
void button_pressedBelowThreshold_Closed();
void button_heldAboveThreshold_Held();
void button_pressed_release_Clicked();
void button_click_getTwice_Open();
void button_heldAboveThreshold_release_Released();
void button_heldUntilLongPressRepeat_LongPressRepeat();
void button_heldUntilLongPressRepeat_keepHeld_Held();
void button_doubleclickWithinTime_doubleClicked();
void button_doubleclickNotWithinTime_Clicked();
void button_longPressRepeatOff_heldUntilLongPressRepeat_Held();
void button_doubleClickOff_doubleclick_Clicked();
// ENCODER
void encoder_constructor_activeLow_setsInputPullup();
void encoder_constructor_activeHigh_setsInput();
void encoder_init_getIncrement0_getAccumulate0();
void encoder_noTurn_getIncrement0();
void encoder_turn1StepClockwise_getIncrement();
void encoder_turn3StepClockwise_getIncrement3_getAccumulate3();
void encoder_turn4StepClockwise_getIncrement4();
void encoder_turn5StepCounterClockwise_getIncrement_getAccumulate_equal();
void encoder_2stepBack2StepForth_getIncrement0();
void encoder_simulateJump0to2_getDecrement2();
void encoder_simulateJump1to3_getDecrement2();
void encoder_acceleration_initMove_wontAccelerate();
void encoder_acceleration_quickTurn();
void encoder_acceleration_slowTurn();
void encoder_moreStepsPerNotch_countsCorrectly();
void encoder_moreStepsPerNotch_acceleratesCorrectly();


#endif // UNITTEST_BUTTON_H