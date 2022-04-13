// Required libraries - don't forget to add them to your IDE!
#include <Arduino.h>
#include <ClickEncoder.h> // by Schallbert
#include <TimerOne.h>
#include <HID-Project.h>
#include <Blink.h>

// ----------------------------------------------------------------------------
// Encoder. A# is the analog pin number on the arduino that would get input from the encoder
//
constexpr uint8_t ENCODER_CLK = A0;
constexpr uint8_t ENCODER_DT = A1;
constexpr uint8_t ENCODER_SW = A2;
constexpr uint8_t ENCODER_STEPSPERNOTCH = 4;
constexpr bool BTN_ACTIVESTATE = LOW;         // Type of encoder button. Either active LOW or active High
constexpr uint16_t ENC_SERVICE_US = 1000;     // 1ms

// static ClickEncoder encoder{ENCODER_CLK, ENCODER_DT, ENCODER_SW, ENCODER_STEPSPERNOTCH, BTN_ACTIVESTATE};
// static TimerOne timer1;
// void timer1_isr();

// ----------------------------------------------------------------------------
// Buttons & pins
//
// void scanPad();
// void scanEncoder();
const int NUMPAD_ROWS = 3;                                         // the size of the button grid: 3x3, 4x2, etc.
const int NUMPAD_COLS = 3;                                         // the size of the button grid: 3x3, 4x2, etc.
int buttonPins[NUMPAD_ROWS*NUMPAD_COLS] = {   4, 3, 2,     // This defines the pins on the arduino
                                              10, 6, 5,      // that will recieve the key presses
                                              9, 7, 8   };                                                                 

// int buttonState[NUMPAD_ROWS*NUMPAD_COLS] = {LOW};
// int prevButtonState[NUMPAD_ROWS*NUMPAD_COLS] = {HIGH};

// ----------------------------------------------------------------------------
// LED - WIP
//
// Blink statusLed(LED_BUILTIN_RX, 250);

// ----------------------------------------------------------------------------
// Command Sets
//
// int Toggle(int, int);
const int numSets = 2;
const int numEncCmd = 5; // command to enter programming mode should be considered seperately, and excluded here
int setSelector = 0; // this determines the default command set that will be used when powered up.

KeyboardKeycode keypadSets[numSets][NUMPAD_ROWS*NUMPAD_COLS] = 
  {
    {KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21}, // Generic example of usually unused keys. works well with AutoHotKey
    {KEY_1, KEY_Q, KEY_F, KEY_D, KEY_4, KEY_W, KEY_B, KEY_E, KEY_R}  // Example League of Legends set ;)
  };

KeyboardKeycode encoderSets[numSets][numEncCmd] =
{
  {KEY_A, KEY_B, KEY_F22, KEY_F23, KEY_F24},
  {KEYPAD_A, KEYPAD_B, KEY_F22, KEY_F23, KEY_F24}
};

// ----------------------------------------------------------------------------
// Editor mode
//
bool editorMode = false;                    //false for release
// bool toggleEditorMode(bool);
// void latencyTest(void (*func)());
int editorCmd = 0;                          // for incoming serial data
constexpr float TEST_REPITITIONS = 100.0;   // number of times to test the function and take the average
//constexpr uint16_t SERIAL_BAUDRATE = 9600;  // unnecessary since hid opens serial port anyway?

// ----------------------------------------------------------------------------
// Keypad rotation - WIP
//
//KeyboardKeycode tempArray[numSets][NUMPAD_ROWS*NUMPAD_COLS];

int currentOrientation = 3; // 0,1,2,3 -> top, left, bottom, right
int lastOrientation = 3;
int delta = currentOrientation - lastOrientation;

// typedef KeyboardKeycode (*accessfn)(int k, int x, int y);

// KeyboardKeycode normal( int k, int x, int y){ return keypadSets[k][(NUMPAD_COLS*x)+y]; }
// KeyboardKeycode rotateCW( int k, int x, int y){ return keypadSets[k][(NUMPAD_COLS-1-x)+3*y]; }   // <<<<<< This is the main thing
// KeyboardKeycode rotateCCW( int k, int x, int y){ return keypadSets[k][3*(NUMPAD_COLS-1-y)+x]; }

// void cloneArray()
// {
//   for (int k = 0; k < numSets; k++)
//     for (int i = 0; i < NUMPAD_ROWS*NUMPAD_COLS; i++)
//       keypadSets[k][i] = tempArray[k][i];
// }

// void rotateMatrix( accessfn afn)
// {
//   for (int k = 0; k < numSets; k++)
//   {
//     for (int x=0; x<NUMPAD_ROWS; x++)
//     {
//       for(int y=0; y<NUMPAD_COLS; y++)
//       {
//         tempArray[k][(NUMPAD_COLS*x)+y] = afn(k,x,y); // now lets try and only pass the pointers
//       }
//     }
//   }
//   cloneArray();
// }

// void initTempArray()
// {
//   for (int k = 0; k < numSets; k++)
//     for (int i = 0; i < NUMPAD_ROWS*NUMPAD_COLS; i++)
//     {
//         tempArray[k][i] = keypadSets[k][i];
//     }
// }

// void printArray()
// {
//   for (int k = 0; k < numSets; k++)
//   {
//     Serial.println("Set " + String(k) + ":");
//     for(int i=0; i<NUMPAD_COLS*NUMPAD_ROWS; i++)
//     {
//       Serial.print(keypadSets[k][i]);
      
//       if((i+1) % NUMPAD_COLS == 0) 
//         Serial.println("");
//       else
//         Serial.print(" ");

//       // delay(100);
//     }
//     Serial.println("");
//   }
//   Serial.println("");
// }

// void checkOrientation()
// { 
//   //currentOrientation = INPUT;
//   delta = currentOrientation - lastOrientation;

//   if(delta != 0)
//   {
//     if(delta > 0)
//     {
//       for(int j=0; j<delta; j++)
//         rotateMatrix(rotateCW);
//       Serial.println("Rotating keypad clockwise");
//       printArray();
//     }
//     else
//     {
//       for(int j=0; j>delta; j--)
//         rotateMatrix(rotateCCW);
//       Serial.println("Rotating keypad counter-clockwise");
//       printArray();
//     }
//   }

//   lastOrientation = currentOrientation;
// }
