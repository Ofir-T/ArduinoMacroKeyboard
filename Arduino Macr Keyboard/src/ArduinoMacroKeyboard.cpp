/*
Hello! this is the Arduino-Pro-Micro Macro-Keyboard code V1 by Ofir Temelman,
originally uploaded to Thingiverse @ https://www.thingiverse.com/thing:4628023. The encoder section of ths code is adapted from Mikolas Zuza's Oversized Volume Knob on Prusaprinters.org
You can use any keyboard buttons available in HID-Project.h (see bottom comment), but I prefer to
keep them the usually unused F13-F24 keys, and change their actions using a macro program
*/

// Required libraries - don't forget to add them to your IDE!
#include <Arduino.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <HID-Project.h>

//Analog pins That would get input from encoder. A# is the pin number on the arduino
#define ENCODER_CLK A0 
#define ENCODER_DT A1
#define ENCODER_SW A2

//Buttons & pins
const int numButtons = 9;
const int buttonPins[9] = {2, 3, 10, 4, 5, 6, 7, 8, 9}; // This defines the pins on the arduino that will recieve the key presses
int buttonState[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // This 
int prevButtonState[9] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};


//Command Sets
const int numSets = 2;
const int numCmd = 14;
int toggle = 0;
KeyboardKeycode commandSets[numSets][numCmd] = 
{ // 9x Button, 2x Encoder rotation, 3x Encoder Click 
  {KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEYPAD_A, KEYPAD_B, KEY_F22, KEY_F23, KEY_F24}, // Fxx + media
  //{KEYPAD_1, KEYPAD_2, KEYPAD_3, KEYPAD_4, KEYPAD_5, KEYPAD_6, KEYPAD_7, KEYPAD_8, KEYPAD_9, KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17}, // Numpad + Fxx
  {KEY_1, KEY_Q, KEY_F, KEY_D, KEY_4, KEY_W, KEY_B, KEY_E, KEY_R, KEYPAD_A, KEYPAD_B, KEY_F22, KEY_F23, KEY_F24}
};

void timerIsr();
int Toggle(int, int);
ClickEncoder *encoder; // variable representing the rotary encoder
int16_t last, value; // variables for current and last rotation value

void setup() //this is a one-time pre-run of stuff we need to get the code running as we planned
{

  //initialize the keypad buttons
  for (int i = 0; i < 9 ; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP); // this goes over every button-pin we defined earlier, and initializes them as input - so they can recieve signals and pullup, so we can connect them directly to the board without resistors
    digitalWrite(buttonPins[i], HIGH);
  } 


// initialize the encoder
   encoder = new ClickEncoder(ENCODER_DT, ENCODER_CLK, ENCODER_SW); // Initializes the rotary encoder with the mentioned pins

// initialize the timer, which the rotary encoder uses to detect rotation
  Timer1.initialize(1000); 
  Timer1.attachInterrupt(timerIsr); 
  last = -1;

  //Serial.begin(9600);
  Keyboard.begin(); //initialize the hid Communication - From this line onwards, the computer should see the arduino as a keyboard
}

void loop() // the main body of our code. this loop runs continuously with our code inside it
{
  // Check if any of the keypad buttons was pressed & send the keystroke if needed
  for (int j = 0; j < 9 ; j++) // goes over every button pin we defined
  {
    buttonState[j] = digitalRead(buttonPins[j]); // reads current button state
    if ((buttonState[j] != prevButtonState[j]) && (buttonState[j] == HIGH)) // if the button changed state, and is now pressed, do what's inside the statement
    {
      Keyboard.press(commandSets[toggle][j]); // this triggers each button's corresponding action e.g. when the 1st button is pressed, the arduino tells the PC that the F13 key was pressed
    }
    prevButtonState[j] = buttonState[j]; // this remebers the button's current state, so we can compare to it on the next round of the loop.
    delay(5);
  }

  //Read the encoder
   value += encoder->getValue();
  
    // Rotation - Volume control
    if (value != last) { // New value is different than the last one, that means to encoder was rotated
      if(last<value) // Detecting the direction of rotation
       Consumer.write(MEDIA_VOLUME_UP); // Replace this line to have a different function when rotating counter-clockwise
        else
        Consumer.write(MEDIA_VOLUME_DOWN); // Replace this line to have a different function when rotating clockwise
      last = value; // Refreshing the "last" varible for the next loop with the current value
    }
  
    // Encoder Clicks - Play/pause, Next
    ClickEncoder::Button b = encoder->getButton(); // Asking the button for it's current state
    if (b != ClickEncoder::Open) { // If the button is unpressed, we'll skip to the end of this if block
      switch (b) {
        case ClickEncoder::Clicked: // Button was clicked once
          Keyboard.press(commandSets[toggle][11]);
          //Consumer.write(MEDIA_PLAY_PAUSE); // Replace this line to have a different function when clicking button once
        break;      
        
        case ClickEncoder::DoubleClicked: // Button was double clicked
          Keyboard.press(commandSets[toggle][12]);
        break;      

        case ClickEncoder::Held: // Button was Held
          toggle = Toggle(toggle, numSets);
          //Keyboard.press(commandSets[toggle][13]);
        break;
      }
    }

    Keyboard.releaseAll();
    delay(10); // a small delay after reading the buttons helps prevent accidental double-presses, like if the arduino reads the button faster than we can release it. 30ms is usually small enough and very reliable // Wait 10 milliseconds, we definitely don't need to detect the rotary encoder any faster than that
}

//Timer for the encoder
void timerIsr() {
  encoder->service();
}

// Multiple Command Sets
int Toggle(int toggle, int numSets) // funtion to toggle between command sets
{
  if(toggle >= 0 && toggle < (numSets-1))
    toggle ++;
  else
    toggle = 0;
  
  return toggle;
}



/*
    This is just a long comment
    Here are some fun functions you can use to replace the default behaviour 
    Consumer.write(CONSUMER_BRIGHTNESS_UP);
    Consumer.write(CONSUMER_BRIGHTNESS_DOWN);
    Consumer.write(CONSUMER_BROWSER_HOME);
    Consumer.write(CONSUMER_SCREENSAVER);
    Consumer.write(HID_CONSUMER_AL_CALCULATOR); //launch calculator :)
    Consumer.write(HID_CONSUMER_AC_ZOOM_IN);
    Consumer.write(HID_CONSUMER_AC_SCROLL_UP);
    CONSUMER_SLEEP = 0x32,

    FULL LIST CAN BE FOUND HERE:
    https://github.com/NicoHood/HID/blob/master/src/HID-APIs/ConsumerAPI.h
*/
