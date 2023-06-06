#pragma once
#ifndef configuration_h
#define configuration_h
#include <Arduino.h>

//Board Type - uncomment the board type you use
// #define BOARD_AVR
#define BOARD_ESP32C3

// Button Layout - comment if your buttons are NOT in a square grid (i.e. NOT 2x2, 3x3 etc.)
#define LAYOUT_SQUARE

// #define ENCODER_CLK D7
// #define ENCODER_DT D8
// #define ENCODER_SW D9

// #define MUX_PIN_1 D1
// #define MUX_PIN_2 D2
// #define MUX_PIN_3 D3
// #define MUX_PIN_4 D4
// #define MUX_PIN_SIG D5
// #define MUX_PIN_EN D6

// #define LED_PIN D10

// #define NUM_ROWS 3                             // the size of the button grid: 3x3, 4x2, etc.
// #define NUM_COLS 3                            // the size of the button grid: 3x3, 4x2, etc.
// #define NUM_BUTTONS NUM_ROWS*NUM_COLS
// #define NUM_SETS 2 // <=16 for Arduino micro because of eeprom size. 
// #define NUM_ENC_CMD 5

#endif