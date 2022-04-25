/*
Hello! this is the Arduino-Macro-Keyboard program,
Project Details on Thingiverse @ https://www.thingiverse.com/thing:4628023.
List of available action is TBD.
by Ofir Temelman 19/04/22.
*/

// Required libraries - don't forget to add them to your IDE!
#include <Arduino.h>
#include <ClickEncoder.h> // by Schallbert
#include <TimerOne.h>
#include <HID-Project.h>
#include <Blink.h>
#include <EEPROM.h>
//#include <RawByteSerial.h>

// ----------------------------------------------------------------------------
// Name. This is the name of your AMK.
//
const char *amkName = "Yo Mama";

// ----------------------------------------------------------------------------
// Encoder. A# is the analog pin number on the arduino that would get input from the encoder
//
constexpr uint8_t ENCODER_CLK = A0;
constexpr uint8_t ENCODER_DT = A1;
constexpr uint8_t ENCODER_SW = A2;
constexpr uint8_t ENCODER_STEPSPERNOTCH = 4;
constexpr boolean BTN_ACTIVESTATE = LOW;         // Type of encoder button. Either active LOW or active High
constexpr uint16_t ENC_SERVICE_US = 1000;     // 1ms

static ClickEncoder encoder{ENCODER_CLK, ENCODER_DT, ENCODER_SW, ENCODER_STEPSPERNOTCH, BTN_ACTIVESTATE};
static TimerOne timer1;
void timer1_isr();

// ----------------------------------------------------------------------------
// Buttons & pins
//
 void scanPad();
//void pressRelease(int (*func)());
//int scanPad();
void scanEncoder();
const int NUM_ROWS = 3;                                         // the size of the button grid: 3x3, 4x2, etc.
const int NUM_COLS = 3;                                         // the size of the button grid: 3x3, 4x2, etc.
int buttonPins[NUM_ROWS*NUM_COLS] = {   4, 3, 2,     // This defines the pins on the arduino, make it a const?
                                        10, 6, 5,      // that will recieve the key presses
                                        9, 7, 8   };                                                                 

int buttonState[NUM_ROWS*NUM_COLS] = {LOW};
int prevButtonState[NUM_ROWS*NUM_COLS] = {HIGH};

// ----------------------------------------------------------------------------
// LED - WIP
//
Blink statusLed(LED_BUILTIN_RX, 250);

// ----------------------------------------------------------------------------
// Command Sets
//
int Toggle(int, int);
const int NUM_SETS = 2;
const int NUM_ENC_CMD = 5; // command to enter programming mode should be considered seperately, and excluded here
int activeSet = 0; // this determines the default command set that will be used when powered up.

KeyboardKeycode keypadSets[NUM_SETS][NUM_ROWS*NUM_COLS] = 
{
  {KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21}, // Generic example of usually unused keys. works well with AutoHotKey
  {KEY_1, KEY_Q, KEY_F, KEY_D, KEY_4, KEY_W, KEY_B, KEY_E, KEY_R}  // Example League of Legends set ;)
};

KeyboardKeycode encoderSets[NUM_SETS][NUM_ENC_CMD] =
{
  {KEY_A, KEY_B, KEY_F22, KEY_F23, KEY_F24},
  {KEYPAD_A, KEYPAD_B, KEY_F22, KEY_F23, KEY_F24}
};

// ----------------------------------------------------------------------------
// Serial communication
//
const int MAX_INPUT_LENGTH = 2 + max((NUM_ROWS*NUM_COLS), (NUM_ENC_CMD)) * NUM_SETS;
char buffer[MAX_INPUT_LENGTH+1], content[MAX_INPUT_LENGTH+1]; // +1 for null terminator of character string
char opCode, header;
int end;
byte endOfLine = '\n'; // as-is, \n collides with keyboardkeycode of 'g', which is also 0x0A

// ----------------------------------------------------------------------------
// Companion app
//
boolean appIsOpen = false;                    
void waitForApp();
constexpr uint16_t SERIAL_BAUDRATE = 9600;  // unnecessary since hid opens serial port anyway?
void readLine();
void sendLayout();
void sendKeyBinding();
void setAppState(boolean);
void sendMessage(char, char, const char *);
void sendMessage(char, char, int);
void setBindings(const char *);

// ----------------------------------------------------------------------------
// Debug Information
//
boolean debugSerialIn = false;
boolean debugSerialOut = false;
const char *opCodeError = "opCodeError";
const char *headerError = "headerError";
const char *unknownError = "unknownError";
const char *lengthError = "lengthError";
const char *contentError = "contentError";
const char *notAFeatureError = "notAFeatureError";
const char *waitingForApp = "waiting for app";
const char *arduinoReady = "AMK ready";
const char *appIsClosing = "app is closing";


void latencyTest(void (*func)());
constexpr float TEST_REPITITIONS = 1.0;//100.0;   // number of times to test the function and take the average
// ----------------------------------------------------------------------------
// Keypad rotation
//
KeyboardKeycode tempArray[NUM_SETS][NUM_ROWS*NUM_COLS];

int currentOrientation = 3; // 0,1,2,3 -> top, left, bottom, right
int lastOrientation = 3;
int delta = currentOrientation - lastOrientation;

typedef KeyboardKeycode (*accessfn)(int k, int x, int y);

KeyboardKeycode normal( int k, int x, int y){ return keypadSets[k][(NUM_COLS*x)+y]; }
KeyboardKeycode rotateCW( int k, int x, int y){ return keypadSets[k][(NUM_COLS-1-x)+3*y]; }   // <<<<<< This is the main thing
KeyboardKeycode rotateCCW( int k, int x, int y){ return keypadSets[k][3*(NUM_COLS-1-y)+x]; }

void cloneArray()
{
  for (int k = 0; k < NUM_SETS; k++)
    for (int i = 0; i < NUM_ROWS*NUM_COLS; i++)
      keypadSets[k][i] = tempArray[k][i];
}

void rotateMatrix( accessfn afn)
{
  for (int k = 0; k < NUM_SETS; k++)
  {
    for (int x=0; x<NUM_ROWS; x++)
    {
      for(int y=0; y<NUM_COLS; y++)
      {
        tempArray[k][(NUM_COLS*x)+y] = afn(k,x,y); // now lets try and only pass the pointers
      }
    }
  }
  cloneArray();
}

void initTempArray()
{
  for (int k = 0; k < NUM_SETS; k++)
    for (int i = 0; i < NUM_ROWS*NUM_COLS; i++)
    {
        tempArray[k][i] = keypadSets[k][i];
    }
}

void printArray()
{
  for (int k = 0; k < NUM_SETS; k++)
  {
    Serial.println("Set " + String(k) + ":");
    for(int i=0; i<NUM_COLS*NUM_ROWS; i++)
    {
      Serial.print(keypadSets[k][i]);
      
      if((i+1) % NUM_COLS == 0) 
        Serial.println("");
      else
        Serial.print(" ");

      // delay(100);
    }
    Serial.println("");
  }
  Serial.println("");
}

void checkOrientation()
{ 
  /*
    INSERT SENEOR READ HERE
  */
 
  delta = currentOrientation - lastOrientation;

  if(delta != 0)
  {
    if(delta > 0)
    {
      for(int j=0; j<delta; j++)
        rotateMatrix(rotateCW);
      // Serial.println("Rotating keypad clockwise");
      // printArray();
    }
    else
    {
      for(int j=0; j>delta; j--)
        rotateMatrix(rotateCCW);
      // Serial.println("Rotating keypad counter-clockwise");
      // printArray();
    }
  }

  lastOrientation = currentOrientation;
}

// ----------------------------------------------------------------------------
// EEPROM - manage long-term memory
// 

/*
Storage Adresses:
  [0 : isFirstBoot ,1-(sizeof(keypadSets)-1) : keypadSets, sizeof(keypadSets)-255: empty]
*/
boolean debugEEPROM = false;
boolean resetAMK = false;
boolean isFirstBoot = (EEPROM.read(0) == 255);
byte isFirstBootAddr = 0;
byte keypadStartAddr = 1;

void printEepromAt(byte addr)//reads the specified EEPROM address and prints result to serial monitor
{
  Serial.print("EEPROM- ");
  byte value = EEPROM.read(addr);
  Serial.print("addr: ");
  Serial.print(addr);
  Serial.print(", ");
  Serial.print("value: ");
  Serial.print(value);
  Serial.println();
}

void saveKeypadToEeprom(KeyboardKeycode bindings[NUM_SETS][NUM_COLS*NUM_ROWS])
{
  byte key;
  byte buttonsPerSet = NUM_COLS*NUM_ROWS;
  byte index;

  if(debugEEPROM)
    Serial.println("Saving keypad to EEPROM");

  for (byte k=0; k < NUM_SETS; k++)
  {
    for(byte i=0; i < buttonsPerSet; i++)
    {
      index = (k*buttonsPerSet)+i+keypadStartAddr;
      key =  bindings[k][i];
      EEPROM.update(index, key);
      printEepromAt(index);
    }
  }
}

void getKeypadTFromEeprom()
{
  byte buttonsPerSet = NUM_COLS*NUM_ROWS;
  byte index;

  for (byte k=0; k < NUM_SETS; k++)
  {
    for(byte i=0; i < buttonsPerSet; i++)
    {
      index = (k*buttonsPerSet)+i+keypadStartAddr;
      keypadSets[k][i] = KeyboardKeycode(EEPROM.read(index));
      // Serial.println(keypadSets[k][i]);
    }
  }
}

// ----------------------------------------------------------------------------
// Initial setup of the encoder, buttons, led, and communication.
// This is a one-time pre-run of stuff we need to get the code running as we planned
//
void setup()
{
  delay(10000);
  if(isFirstBoot || resetAMK)
  {
    Serial.println("Resetting keypad to default");
    saveKeypadToEeprom(keypadSets);
    getKeypadTFromEeprom();
    EEPROM.update(0, 1);

    if(debugEEPROM)
    {
      Serial.print("isFirstBoot: ");
      Serial.println(EEPROM.read(0));
    }
  }
  else
  {
    Serial.println("Retrieveing keypad");
    getKeypadTFromEeprom();
    printArray();
  }

  pinMode(LED_BUILTIN_TX, INPUT);               // this is a trick to turn off the tx/rx onboard leds
  //pinMode(LED_BUILTIN_RX, INPUT);

//initialize the keypad buttons
  for (int i = 0; i < NUM_ROWS*NUM_COLS ; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP); // this goes over every button-pin we defined earlier, and initializes them as input - 
    digitalWrite(buttonPins[i], HIGH);    // so they can recieve signals and pullup, so we can connect them directly to the board without resistors
  }

  initTempArray();
  buffer[0] = 0;

// Encoder features
  encoder.setAccelerationEnabled(true); // Comment/delete this line to disable acceleration. Adjusting acceleration values can be done in the ClickEncoder.h file
  encoder.setDoubleClickEnabled(true);
  encoder.setLongPressRepeatEnabled(true);

// initialize the timer, which the rotary encoder uses to detect rotation
  timer1.attachInterrupt(timer1_isr);
  timer1.initialize(ENC_SERVICE_US);

//initialize the hid Communication - From the next line onwards, the computer should see the arduino as a keyboard
  Keyboard.begin(); 
}

// ----------------------------------------------------------------------------
// the main body of our code. this loop runs continuously with our code inside it
//
void loop() 
{
//---------------------------------THIS---------------------------------

  if (Serial.available() > 0) // Check for messages from the app
  {
    if(appIsOpen)
        readLine();
    else
      waitForApp();
  }
  else // otherwise, keep scanning the buttons
  {
    scanPad();
    scanEncoder();
    checkOrientation();
  }
}

// ----------------------------------------------------------------------------
// Core Functions
//
void scanPad() // Check if any of the keypad buttons was pressed & send the keystroke if needed
{
  for (int i = 0; i < NUM_ROWS*NUM_COLS ; i++) // goes over every button pin we defined
  {
    buttonState[i] = digitalRead(buttonPins[i]); // reads current button state
    if ((buttonState[i] != prevButtonState[i])) // button changed state
    {
      if(buttonState[i] == LOW) // button was pressed
      {
        if(appIsOpen)
          sendMessage('p', 'k', i); // send the index of the button that was pressed, blocks the key action.
        else
          Keyboard.press(keypadSets[activeSet][i]); // send the button's action
      }
      else // button was released
        Keyboard.release(keypadSets[activeSet][i]); // release the button
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
      if(lastValue>value) // Detecting the direction of rotation
      { //clockwise rotation
        Consumer.write(MEDIA_VOLUME_UP); // Replace this line to have a different function when rotating counter-clockwise
        //Keyboard.press(encoderSets[activeSet][0]);
        Keyboard.releaseAll();
      }
      else
      { // counter-clockwise rotation
        Consumer.write(MEDIA_VOLUME_DOWN); // Replace this line to have a different function when rotating clockwise
        //Keyboard.press(encoderSets[activeSet][1]);
        Keyboard.releaseAll();
      }
    }
    lastValue = value;
    
    switch (encoder.getButton()) // Encoder Clicks
    {
    case Button::Clicked: // single click
        Keyboard.press(encoderSets[activeSet][2]);
        Keyboard.releaseAll();
        break;
    case Button::DoubleClicked: // double click
        currentOrientation = Toggle(currentOrientation, 4);
        checkOrientation();
        sendMessage('s', 'o', currentOrientation);
        break;
    case Button::LongPressRepeat: // right now, getting to long press repeat also triggers held command
        break;
    case Button::Held:
        if (!appIsOpen)
        {
          activeSet = Toggle(activeSet, NUM_SETS);
          statusLed.sequence(3, 150);
        }
        // if (appIsOpen)
        //     Serial.println("key set: " + String(activeSet));
        break;
    case Button::Released:
        break;
    default:
        // no output for "Open" or "Closed" to not spam the console.
        break;
    }

  //delay(0.5); // a small delay after reading the buttons helps prevent accidental double-presses. Un-comment if you're having ghosting issues.
}

void timer1_isr()
{
    // This is the Encoder's worker routine. It will physically read the hardware.
    // and all most of the logic happens here. Recommended interval for this method is 1ms.
    encoder.service();
}

// returns selector+1 over specified range. i.e. 0->1, 1->2,...,(range-2)->(range-1), (range-1)->0,...
int Toggle(int selector, int range)
{
  if(selector >= 0 && selector < (range-1))
    selector ++;
  else
    selector = 0;
  
  return selector;
}

//Start app communication
void waitForApp()
{
  // read a 'line' from the serial into buffer, then split it to opCode, header, and content
  end = Serial.readBytesUntil('\n', buffer, MAX_INPUT_LENGTH+2);
  buffer[end] = '\0';
  opCode = buffer[0];
  header = buffer[1];
  // something to do if buffer has less than 3 bytes in it. i.e. end < 3 ?

  //not reading content because it is not important in this instance

  // if the right message was recieved, signal ready
  if(opCode == 'a' && header == 'o')
  {
    appIsOpen = true;
    sendMessage('p', 'r', arduinoReady);
  }
  else
    sendMessage('p', 'm', waitingForApp);
}

//measures execution time in milliseconds for any given void func(). used to estimate the keyboards latency.
void latencyTest(void (*func)())
{
  unsigned long startMillis = millis();
    for (int n = 0; n < TEST_REPITITIONS; n++) {
      (*func)();
    }
    unsigned long endMillis = millis();
    float delta = (endMillis - startMillis)/TEST_REPITITIONS;
    Serial.println("Execution time: " + String(delta, 6) + "ms");
}

void readLine()
{ 
  // read a 'line' from the serial into buffer, then split it to opCode, header, and content
  end = Serial.readBytesUntil('\n', buffer, MAX_INPUT_LENGTH+5);
  buffer[end] = '\0';
  opCode = buffer[0];
  header = buffer[1];
  //byte length = buffer[2]; //ignoring length for the time being
  // something to do if buffer has less than 3 bytes in it. i.e. end < 3?
  
  for(byte i=2; i < strlen(buffer); i++)
  {
    content[i-2] = buffer[i]; //-'0;
  }

  if(debugSerialIn == true) // "return to sender"
      sendMessage('p', header, content);

  // process message and choose corresponding action
  switch(opCode)
  {
    case 'a': // app related requests
      if(header == 'o')
        sendMessage('e', header, arduinoReady);
      else
        if(header == 'c')
        {
          sendMessage('p', opCode, appIsClosing);
          appIsOpen = false;
        }
      break;
    case 'g': // get requests
      switch(header)
      {
        case 'e':
          sendMessage('p', header, appIsOpen);
          break;
        case 'b':
          sendKeyBinding();
          break;
        case 'c':
          sendLayout();
          break;
        case 'o':
          sendMessage('s', header, currentOrientation);
          // sendKeyBinding();
          break;
        case 'a':
          sendMessage('s', 'a', activeSet);
          break;
        case 'n':
          sendMessage('s', 'n', amkName);
          break;
        default:
        header = (header < '0') ? 'u' : header;
        sendMessage('e', header, headerError);
      }
      break;//end get requests

    case 's': // set requests 
      switch(header)
        {

          case 'b':
            setBindings(content);
            sendKeyBinding();
            saveKeypadToEeprom(keypadSets);
            break;
          case 'c':
            sendMessage('e', header, headerError);
            break;
          case 'o':
            currentOrientation = atoi(content);
            checkOrientation();
            sendMessage('s', header, currentOrientation);
            //sendKeyBinding();
            break;
          default:
            header = (header < '0') ? 'u' : header;
            sendMessage('e', header, headerError);
        }
        break;  //end set requests

    case 't': // test requests
      switch(header)
      {
        case 'l':
          latencyTest(scanPad);
          latencyTest(scanEncoder);
          sendMessage('e', header, "<latency test results>");
          break;
        case 'o':
          sendMessage('s', header, currentOrientation);
          //sendKeyBinding();
          break;
        default:
        header = (header < '0') ? 'u' : header;
        sendMessage('e', header, headerError);
      }
      break;//end test requests
    break; // end processing requests
    default:
        sendMessage('e', header, opCodeError);
  }
  
  if(debugSerialOut == true) // "return to sender"
    sendMessage('p', header, content);
} // end readline

// sends to serial a message in the defined format. for message that is a character or a character string
void sendMessage(char opCode, char header, const char *content)
{
  byte length = strlen(content);

  Serial.write(opCode);
  Serial.write(header);
  Serial.write(length); // length of message in bytes
  Serial.write(content);
  Serial.write('\n');
}

// sends to serial a message in the defined format. for messages that are an integer
void sendMessage(char opCode, char header, int message)
{
  itoa(message, content, 10);
  byte length = strlen(content);

  Serial.write(opCode);
  Serial.write(header);
  Serial.write(length);// length of message in bytes
  Serial.write(content);
  Serial.write('\n');
}

void sendLayout() // sends the physical layout of the keyboard: number of key rows, number of key columns, number of key sets.
{
  sprintf(content, "%d%d%d",NUM_ROWS, NUM_COLS, NUM_SETS);
  sendMessage('s', 'c', content);
}

void sendKeyBinding() //message format: 
{ 
  int buttonsPerSet = NUM_COLS*NUM_ROWS;
  for (int k = 0; k < NUM_SETS; k++)
  {
    for(int i=0; i<NUM_COLS*NUM_ROWS; i++)
    {
      content[(k*buttonsPerSet) + i] = keypadSets[k][i];
    }
  }
  sendMessage('s', 'b', content);
}

void setAppState(boolean value) // unused right now
{
  appIsOpen = value;
}

// sets the AMK's keybindings to those conatined in content (c-string).
void setBindings(const char *content)
{
  int buttonsPerSet = NUM_COLS*NUM_ROWS;
  for (int k = 0; k < NUM_SETS; k++)
  {
    for(int i=0; i<buttonsPerSet; i++)
    {
      keypadSets[k][i] = KeyboardKeycode(content[(k*buttonsPerSet) + i]);
    }
  }
}

// ----------------------------------------------------------------------------
// Work In Progress
//

// void keypadSetsup()
// {
//   //Print:
//   // switch testing sequence initiated, please press the keypad buttons in the following order:
//   // [1] [2] [3]
//   // [4] [5] [6]
//   // [7] [8] [9]

//   Serial.println("Switch testing sequence initiated, please press the keypad buttons in order:");

//   for (int i = 0; i < NUM_ROWS ; i++)
//   {
//     for(int j = 0; j < NUM_COLS; j++)
//     {
//       Serial.print("[" + String((j+1) + (i*NUM_COLS)) + "] ");
//     }
//     Serial.print("\n");
//   }

//   // test pins
//   int newPinsArray[NUM_ROWS*NUM_COLS];
//   for (int button = 0; button < NUM_ROWS * NUM_COLS ; button++)
//   {
//     newPinsArray[button] = findPin();
//     Serial.print("[" + newPinsArray[button] + "] ");
//     if (button % NUM_COLS == 0 )
//       Serial.print("\n");
//   }

//   Serial.println("Do you want to update the pins with the above order? (y/n)");
//   while (Serial.available() == 0)
//   {

//   }
  
//   switch (Serial.read())
//     {
//       case 'y':
//         buttonPins = newPinsArray;
//         Serial.println("Updated!");
//         break;
//       case 'n':
//         Serial.println("OK. Skipping.");
//         break;
//       default:
//         Serial.println("Unfamiliar command. Skipping process")
//     }

// }

// int findPin() // Scans the buttons until one is pressed. blocks all other functionality
// { 
//   while(true)
//   {
//     for (int i = 0; i < NUM_ROWS * NUM_COLS ; i++) // goes over every button pin we defined
//     {
//       buttonState[i] = digitalRead(buttonPins[i]); // reads current button state
//       if ((buttonState[i] != prevButtonState[i])) // if the button changed state, and is now pressed, do what's inside the statement
//       {
//         if(buttonState[i] == LOW)
//           return buttonPins[i];
//       }
//       prevButtonState[i] = buttonState[i]; // this remebers the button's current state, so we can compare to it on the next round of the loop.
//     } 
//   }
// }


// void rotator(int buttonPins, byte wantedOrientation)
// {
  
// }
//----------------------------------------------------------------------------------------
//experimental scanning function

// void scanPad(void (*func)(int button)) // Check if any of the keypad buttons was pressed & send the keystroke if needed
// {
//   for (int i = 0; i < NUM_ROWS * NUM_COLS ; i++) // goes over every button pin we defined
//   {
//     buttonState[i] = digitalRead(buttonPins[i]); // reads current button state
//     if ((buttonState[i] != prevButtonState[i])) // if the button changed state, and is now pressed, do what's inside the statement
//     {
//       (*func)(i);
//     }
//     prevButtonState[i] = buttonState[i]; // this remebers the button's current state, so we can compare to it on the next round of the loop.
//   }
// }
// Exclusive Button Hold: a flag/ function of holding a button prevents the inital click from actuating

// void pressRelease()
// {
//   int button = scanPad();
//   if(buttonState[button] == LOW)
//     Keyboard.press(keypadSets[activeSet][button]);
//   else
//     Keyboard.release(keypadSets[activeSet][button]);
// }

// int getPin(int button)
// {
//   if(buttonState[button] == LOW)
//     return buttonPins[button];
// }

// void rotateKeypad(int buttonPins, int lastOrientation)
// {
//   int currentOrientation = getOrientation();
//   switch(currentOrientation-lastOrientation)
//   {
//     case 0:
//       break;
//     case 1:

//   }
// }