/*

*/
//#include <Keyboard.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <HID-Project.h>

#define ENCODER_CLK A0 // Change A0 to, for example, A5 if you want to use analog pin 5 instead
#define ENCODER_DT A1
#define ENCODER_SW A2
//
// Function Keys
#define KEYCODE_F13 0xF0 //0x68 & 0x88
#define KEYCODE_F14 0xF1 //0x69 & 0x88
#define KEYCODE_F15 0xF2 //0x6A & 0x88
#define KEYCODE_F16 0xF3 //0x6B & 0x88
#define KEYCODE_F17 0xF4 //0x6C & 0x88
#define KEYCODE_F18 0xF5 //0x6D & 0x88
#define KEYCODE_F19 0xF6 //0x6E & 0x88
#define KEYCODE_F20 0xF7 //0x6F & 0x88
#define KEYCODE_F21 0xF8 //0x70 & 0x88
#define KEYCODE_F22 0xF9 //0x71 & 0x88
#define KEYCODE_F23 0xFA //0x72 & 0x88
#define KEYCODE_F24 0xFB //0x73 & 0x88
//#define KEY_RIGHT_ALT  0x86

 
ClickEncoder *encoder; // variable representing the rotary encoder
int16_t last, value; // variables for current and last rotation value


const int buttonPins[9] = {2, 3, 10, 4, 5, 6, 7, 8, 9};
int buttonState[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int prevButtonState[9] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
bool flag = false;
byte counter = 0x39;

byte commandSet1[13] = {KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24, KEY_RIGHT_ALT };
//int[] CommandSets[4] = {CommandSet1[13], null, null, null};
//int toggle[4] = {0, 1, 2, 3};

void setup()
{
  for (int i = 0; i < 9 ; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
    digitalWrite(buttonPins[i], HIGH);
  }

   encoder = new ClickEncoder(ENCODER_DT, ENCODER_CLK, ENCODER_SW); // Initializes the rotary encoder with the mentioned pins

  Timer1.initialize(1000); // Initializes the timer, which the rotary encoder uses to detect rotation
  Timer1.attachInterrupt(timerIsr); 
  last = -1;

  //Serial.begin(9600);
  Keyboard.begin();
  //Consumer.begin();

}

void loop()
{
    // Buttons
  for (int j = 0; j < 9 ; j++)
  {
    //Serial.println(buttonPins[j]);
    buttonState[j] = digitalRead(buttonPins[j]);
    if ((buttonState[j] != prevButtonState[j]) && (buttonState[j] == HIGH))
    {
      switch (j)
      {
        case 0:
        Keyboard.press(KEY_F13);
        break;

        case 1:
        Keyboard.press(KEY_F14);
        break;

        case 2:
        Keyboard.press(KEY_F15);
        break;

        case 3:
        Keyboard.press(KEY_F16);
        break;

        case 4:
        Keyboard.press(KEY_F17);
        break;

        case 5:
        Keyboard.press(KEY_F18);
        break;

        case 6:
        Keyboard.press(KEY_F19);
        break;

        case 7:
        Keyboard.press(KEY_F20);
        break;

        case 8:
        Keyboard.press(KEY_F21);
        break;
      }
      //Keyboard.press(commandSet1[j]);
      //Serial.println(commandSet1[j]); 
      delay(30);
    }
    prevButtonState[j] = buttonState[j];
  }


  // Encoder
   value += encoder->getValue();
  
    // Rotation
    if (value != last) { // New value is different than the last one, that means to encoder was rotated
      if(last<value) // Detecting the direction of rotation
       //Keyboard.press(commandSet1[9]); 
       Consumer.write(MEDIA_VOLUME_UP); // Replace this line to have a different function when rotating counter-clockwise
        else
        //Keyboard.press(commandSet1[10]);
        Consumer.write(MEDIA_VOLUME_DOWN); // Replace this line to have a different function when rotating clockwise
      last = value; // Refreshing the "last" varible for the next loop with the current value
      //Serial.print("Encoder Value: "); // Text output of the rotation value used manily for debugging (open Tools - Serial Monitor to see it)
      //Serial.println(value);
    }
  
    // Encoder Clicks
    ClickEncoder::Button b = encoder->getButton(); // Asking the button for it's current state
    if (b != ClickEncoder::Open) { // If the button is unpressed, we'll skip to the end of this if block
      //Serial.print("Button: "); 
      //#define VERBOSECASE(label) case label: Serial.println(#label); break;
      switch (b) {
        case ClickEncoder::Clicked: // Button was clicked once
          //Keyboard.press(commandSet1[11]);
          Consumer.write(MEDIA_PLAY_PAUSE); // Replace this line to have a different function when clicking button once
        break;      
        
        case ClickEncoder::DoubleClicked: // Button was double clicked
           //Keyboard.press(commandSet1[12]);
           Consumer.write(MEDIA_NEXT); // Replace this line to have a different function when double-clicking
        break;      
      }
    }

    Keyboard.releaseAll();
    delay(10); // Wait 10 milliseconds, we definitely don't need to detect the rotary encoder any faster than that
}

int Toggle(String action)
{
  // funtion to toggle between command sets
}

void timerIsr() {
  encoder->service();
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
