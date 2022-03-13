/*
Hello! this is the Arduino-Pro-Micro Macro-Keyboard code V1 by Ofir Temelman,
originally uploaded to Thingiverse @ https://www.thingiverse.com/thing:4628023.
You can use any keyboard buttons available in HID-Project.h (see bottom comment)
*/

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

static ClickEncoder encoder{ENCODER_CLK, ENCODER_DT, ENCODER_SW, ENCODER_STEPSPERNOTCH, BTN_ACTIVESTATE};
static TimerOne timer1;
void timer1_isr();

// ----------------------------------------------------------------------------
// Buttons & pins
//
void scanPad();
void scanPad(void (*)());
void scanEncoder();
const int NUMPAD_ROWS = 3;                                         // the size of the button grid: 3x3, 4x2, etc.
const int NUMPAD_COLS = 3;                                         // the size of the button grid: 3x3, 4x2, etc.
int buttonPins[NUMPAD_ROWS*NUMPAD_COLS] = {   2, 3, 10,     // This defines the pins on the arduino
                                              4, 5, 6,      // that will recieve the key presses
                                              7, 8, 9   };                                                                 

int buttonState[NUMPAD_ROWS*NUMPAD_COLS] = {  0, 0, 0, // change to LOW instead?
                                              0, 0, 0,
                                              0, 0, 0   };

int prevButtonState[NUMPAD_ROWS*NUMPAD_COLS] = {  HIGH, HIGH, HIGH,
                                                  HIGH, HIGH, HIGH,
                                                  HIGH, HIGH, HIGH   };

// ----------------------------------------------------------------------------
// LED - WIP
//
Blink statusLed(LED_BUILTIN_RX, 250);

// ----------------------------------------------------------------------------
// Command Sets
//
int Toggle(int, int);
const int numSets = 2;
const int numEncCmd = 5; // command to enter programming mode should be considered seperately, and excluded here
int setSelector = 0; // this determines the default command set that will be used when powered up.

KeyboardKeycode keyPadSets[numSets][NUMPAD_ROWS*NUMPAD_COLS] = 
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
bool toggleEditorMode(bool);
void executionTime(void (*)());
int editorCmd = 0;                          // for incoming serial data
constexpr float TEST_REPITITIONS = 100.0;   // number of times to test the function and take the average
constexpr uint16_t SERIAL_BAUDRATE = 9600;  // unnecessary since hid opens serial port anyway?

// ----------------------------------------------------------------------------
// Initial setup of the encoder, buttons, led, and communication.
// This is a one-time pre-run of stuff we need to get the code running as we planned
//
void setup()
{
  pinMode(LED_BUILTIN_TX, INPUT);               // this is a trick to turn off the tx/rx onboard leds
  //pinMode(LED_BUILTIN_RX, INPUT);

//initialize the keypad buttons
  for (int i = 0; i < NUMPAD_ROWS*NUMPAD_COLS ; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP); // this goes over every button-pin we defined earlier, and initializes them as input - 
    digitalWrite(buttonPins[i], HIGH);    // so they can recieve signals and pullup, so we can connect them directly to the board without resistors
  } 

// Encoder features
  encoder.setAccelerationEnabled(true); // Comment/delete this line to disable acceleration. Adjusting acceleration values can be done in the ClickEncoder.h file
  encoder.setDoubleClickEnabled(true);
  encoder.setLongPressRepeatEnabled(true);

// initialize the timer, which the rotary encoder uses to detect rotation
  timer1.attachInterrupt(timer1_isr);
  timer1.initialize(ENC_SERVICE_US);

  Keyboard.begin(); //initialize the hid Communication - From this line onwards, the computer should see the arduino as a keyboard

  delay(5000); // 5 seconds of mercy
}

// ----------------------------------------------------------------------------
// the main body of our code. this loop runs continuously with our code inside it
//
void loop() 
{
  if (editorMode && Serial.available() > 0) // Check for editor commands
  {
    editorCmd = Serial.read(); // read the incoming byte:
    switch (editorCmd) //check if it matches any of the editor mode comannds
    {
      case 'l':
        executionTime(scanPad);
        executionTime(scanEncoder);
        break;
      case 's':
        //keypadSetup();
        break;
        //editorCmd = 0; // happens if matches any of the above cases
      default:
        break;
    }
  }
  else // otherwise, keep scanning the buttons
  {
    scanPad();
    scanEncoder();
  }
}

// ----------------------------------------------------------------------------
// Helper Functions
//
void scanPad() // Check if any of the keypad buttons was pressed & send the keystroke if needed
{
  for (int i = 0; i < NUMPAD_ROWS*NUMPAD_COLS ; i++) // goes over every button pin we defined
  {
    buttonState[i] = digitalRead(buttonPins[i]); // reads current button state
    if ((buttonState[i] != prevButtonState[i])) // if the button changed state, and is now pressed, do what's inside the statement
    {
      if(buttonState[i] == LOW)
      { // action to take after press:
        if(!editorMode)
          Keyboard.press(keyPadSets[setSelector][i]); // this triggers each button's corresponding action e.g. when the 1st button is pressed, the arduino tells the PC that the F13 key was pressed
        else  // for debugging
          Serial.println("This button is connected to pin " + String(buttonPins[i]));;
      }
      else
        Keyboard.releaseAll();
    }
    prevButtonState[i] = buttonState[i]; // this remebers the button's current state, so we can compare to it on the next round of the loop.
  } 

  //delay(5); // a small delay after reading the buttons helps prevent accidental double-presses.
}

void scanEncoder()
{
  static int16_t lastValue{0};
    int16_t value = encoder.getAccumulate();
    if (value != lastValue) // Encoder Rotation
    {
      if(lastValue>value){ // Detecting the direction of rotation
        Consumer.write(MEDIA_VOLUME_UP); // Replace this line to have a different function when rotating counter-clockwise
        //Keyboard.press(encoderSets[setSelector][0]);
        Keyboard.releaseAll();
      }
      else{
        Consumer.write(MEDIA_VOLUME_DOWN); // Replace this line to have a different function when rotating clockwise
        //Keyboard.press(encoderSets[setSelector][1]);
        Keyboard.releaseAll();
      }
    }
    lastValue = value;
    
    switch (encoder.getButton()) // Encoder Clicks
    {
    case Button::Clicked:
        Keyboard.press(encoderSets[setSelector][2]);
        Keyboard.releaseAll();
        break;
    case Button::DoubleClicked:
        Keyboard.press(encoderSets[setSelector][3]);
        Keyboard.releaseAll();
        break;
    case Button::Held:
        setSelector = Toggle(setSelector, numSets);
        statusLed.sequence(3, 150);
        if (editorMode)
            Serial.println("key set: " + String(setSelector));
        break;
    case Button::LongPressRepeat: // right now, getting to long press repeat also triggers held command
        editorMode = toggleEditorMode(editorMode);
        Serial.println("Editor Mode:" + String(editorMode));
        delay(1000);// delay to prevent repeated toggling of editor mode. maybe insert to function?
        break;
    case Button::Released:
        break;
    default:
        // no output for "Open" or "Closed" to not spam the console.
        break;
    }

  //delay(5); // a small delay after reading the buttons helps prevent accidental double-presses.
}

void timer1_isr()
{
    // This is the Encoder's worker routine. It will physically read the hardware
    // and all most of the logic happens here. Recommended interval for this method is 1ms.
    encoder.service();
}

//Command Sets selector
int Toggle(int selector, int numSets)
{
  if(selector >= 0 && selector < (numSets-1))
    selector ++;
  else
    selector = 0;
  
  return selector;
}

//Start-Stop editor mode (toggles serial communication, enables/disables latency testing & key remapping)
bool toggleEditorMode(bool editorMode)
{
  if(!editorMode)
  {
    //Serial.begin(SERIAL_BAUDRATE);
    //editing actions to be placed here
    editorMode = true;
  }
  else
  {
    //save edits & close serial communication
    //Serial.end();
    editorMode = false;
  }
  return editorMode;
}

//measures execution time in milliseconds for any given void func(). used to estimate the keyboards latency.
void executionTime(void (*func)())
{
  unsigned long startMillis = millis();
    for (int n = 0; n < TEST_REPITITIONS; n++) {
      (*func)();
    }
    unsigned long endMillis = millis();
    float delta = (endMillis - startMillis)/TEST_REPITITIONS;
    Serial.println("Execution time: " + String(delta, 6) + "ms");
}

// ----------------------------------------------------------------------------
// Work In Progress
//

void keypadSetup()
{
  //Print:
  // switch testing sequence initiated, please press the keypad buttons in the following order:
  // [1] [2] [3]
  // [4] [5] [6]
  // [7] [8] [9]

  Serial.println("Switch testing sequence initiated, please press the keypad buttons in order:");

  for (int i = 0; i < NUMPAD_ROWS ; i++)
  {
    for(int j = 0; j < NUMPAD_COLS; j++)
    {
      Serial.print("[" + String((j+1) + (i*NUMPAD_COLS)) + "] ");
    }
    Serial.print("\n");
  }

  // test pins
  int newPinsArray[NUMPAD_ROWS*NUMPAD_COLS];
  for (int button = 0; button < NUMPAD_ROWS * NUMPAD_COLS ; button++)
  {
    newPinsArray[button] = findPin();
    Serial.print("[" + newPinsArray[button] + "] ")
    if (button % NUMPAD_COLS == 0 )
      Serial.print("\n");
  }

  Serial.println("Do you want to update the pins with the above order? (y/n)");
  while (Serial.available() == 0)
  {

  }
  
  switch (Serial.read())
    {
      case 'y':
        buttonPins = newPinsArray;
        Serial.println("Updated!");
        break;
      case 'n':
        Serial.println("OK. Skipping.");
        break;
      default:
        Serial.println("Unfamiliar command. Skipping process")
    }

}

int findPin() // Scans the buttons until one is pressed. blocks all other functionality
{ 
  while(true)
  {
    for (int i = 0; i < NUMPAD_ROWS * NUMPAD_COLS ; i++) // goes over every button pin we defined
    {
      buttonState[i] = digitalRead(buttonPins[i]); // reads current button state
      if ((buttonState[i] != prevButtonState[i])) // if the button changed state, and is now pressed, do what's inside the statement
      {
        if(buttonState[i] == LOW)
          return buttonPins[i];
      }
      prevButtonState[i] = buttonState[i]; // this remebers the button's current state, so we can compare to it on the next round of the loop.
    } 
  }
}


void rotator(int buttonPins, byte wantedOrientation)
{
  
}
//----------------------------------------------------------------------------------------
//experimental scanning function

// void scanPad(void (*func)(int button)) // Check if any of the keypad buttons was pressed & send the keystroke if needed
// {
//   for (int i = 0; i < NUMPAD_ROWS * NUMPAD_COLS ; i++) // goes over every button pin we defined
//   {
//     buttonState[i] = digitalRead(buttonPins[i]); // reads current button state
//     if ((buttonState[i] != prevButtonState[i])) // if the button changed state, and is now pressed, do what's inside the statement
//     {
//       (*func)(i);
//     }
//     prevButtonState[i] = buttonState[i]; // this remebers the button's current state, so we can compare to it on the next round of the loop.
//   } 
// // Exclusive Button Hold: a flag/ function of holding a button prevents the inital click from actuating

// void pressOrRelease(int button)
// {
//   if(buttonState[button] == LOW)
//     Keyboard.press(keyPadSets[setSelector][button]);
//   else
//     Keyboard.release(keyPadSets[setSelector][button]);
// }

// int getPin(int button)
// {
//   if(buttonState[button] == LOW)
//     return buttonPins[button]);
// }

