/*
Hello! this is the Arduino-Macro-Keyboard program,
Project Details on Thingiverse @ https://www.thingiverse.com/thing:4628023.
List of available actions is TBD.
by Ofir Temelman 19/04/22.
*/

// version number: 2.x
// version date: 10082022

// Required libraries - don't forget to add them to your IDE!
#include <Arduino.h>
// #include "Configuration.h"
#include <ClickEncoder.h> // by Schallbert
#include <TimerOne.h>
#include <HID-Project.h>
#include <Blink.h>
#include <EEPROM.h>
#include <NonBlockingSerial.h>
// #include <ArduinoPC.h>
#include <Command.h>

// ----------------------------------------------------------------------------
// Name. This is the name of your AMK.
//
const char amkName[] PROGMEM = "Yo Mama";

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
constexpr int NUM_ROWS = 3;                                         // the size of the button grid: 3x3, 4x2, etc.
constexpr int NUM_COLS = 3;                                         // the size of the button grid: 3x3, 4x2, etc.
constexpr int buttonPins[NUM_ROWS*NUM_COLS] = {   4, 3, 2,     // This defines the pins on the arduino, make it a const?
                                        10, 6, 5,      // that will recieve the key presses
                                        9, 7, 8   };                                                                 

int buttonState[NUM_ROWS*NUM_COLS] = {LOW};
int prevButtonState[NUM_ROWS*NUM_COLS] = {HIGH};
boolean encoderHeld = false;

void scanPad();
void scanEncoder();
// ----------------------------------------------------------------------------
// LED - WIP
//
// Blink statusLed(LED_BUILTIN_RX, 250);

// ----------------------------------------------------------------------------
// Command Sets
//
int Toggle(int, int);
constexpr int NUM_SETS = 2; // <=16 for Arduino micro because of eeprom size. 
constexpr int NUM_ENC_CMD = 5;
int activeSet = 0; // this determines the default command set that will be used when powered up.

// NTS: change defaults to something more general that can scale with grid size.
KeyboardKeycode keypadSets[NUM_SETS][NUM_ROWS*NUM_COLS] = 
{
  {KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21}, // Generic example of usually unused keys. works well with AutoHotKey
  {KEY_1, KEY_Q, KEY_F, KEY_D, KEY_4, KEY_W, KEY_B, KEY_E, KEY_R}  // Example League of Legends set :)
};

KeyboardKeycode encoderSets[NUM_SETS][NUM_ENC_CMD] =
{
  {KEY_A, KEY_B, KEY_F22, KEY_F23, KEY_F24},
  {KEYPAD_A, KEYPAD_B, KEY_F22, KEY_F23, KEY_F24}
};

Command *keypadSet[NUM_SETS][NUM_ROWS*NUM_COLS];
Command *tmpArray[NUM_SETS][NUM_ROWS*NUM_COLS];
Command *encoderSet[NUM_SETS][NUM_ENC_CMD];

// ----------------------------------------------------------------------------
// Memory - SRAM & EEPROM
// 
/*
  An Arduino Pro Micro has 1024 Bytes of EEPROM memory, and 2560 bytes of SRAM.
  Command memory usage is as follows:
  Chr - 2 Bytes
  Cons - 2 Bytes
  CStr - length + 2 Bytes

Storage Addresses:
  [0: FirstBoot, 64->1024: Commands]

*/

const uint16_t reservedMemory = 64; // for future use
constexpr uint16_t reservedRam = 1200; // according to rest of the program
constexpr uint16_t keysetMem = reservedRam/4;
constexpr uint16_t commandMemory = 1024-reservedMemory;
const uint16_t sizePerSet = (960/NUM_SETS)-(NUM_ROWS*NUM_COLS)-NUM_ENC_CMD-1; // subtract null terminators
uint16_t availableCommandMemory = 960;
constexpr uint16_t BUFFER_SIZE = keysetMem+5; // +startMarker, endMarker, length, tag(2B)
constexpr byte MAX_COMMAND_SIZE = Command::STRING_LENGTH+2; // 1B type, 1B seperator. Change STRING_LENGTH in Command.h to change this
constexpr uint16_t MAX_COMMANDS = commandMemory/MAX_COMMAND_SIZE; //(useMemory) ? ((EEPROM.length()-reservedMemory)/MAX_STRING_LENGTH):(960/MAX_STRING_LENGTH);

boolean debugEEPROM = true;
boolean useMemory = true; // set to false if you don't want to store data in EEPROM. //something about setting this to true crashes the arduino on startup.

// constexpr int numcommands = NUM_SETS*(NUM_ENC_CMD + (NUM_ROWS*NUM_COLS));
// static_assert((numcommands <= MAX_COMMANDS), "Number of commands defined exceeds reserved memory");

void printEepromAt(int addr);
void saveKeypadToEeprom(KeyboardKeycode bindings[NUM_SETS][NUM_COLS*NUM_ROWS]);
void getKeypadTFromEeprom();

// ----------------------------------------------------------------------------
// Serial communication
//
constexpr long SERIAL_BAUDRATE = 57600;  // unnecessary since hid opens serial port anyway?
byte inBuffer[BUFFER_SIZE], outBuffer[BUFFER_SIZE], tmpBuffer[BUFFER_SIZE];
NonBlockingSerial2 nBSerial(BUFFER_SIZE, inBuffer, outBuffer, tmpBuffer);
char buffer[1], content[1]; // +1 for null terminator of character string
char message[48]; //for general messages
char opCode, header;
int end;

// ----------------------------------------------------------------------------
// Companion app
//

boolean appIsOpen = true;                    
void waitForApp();
void readLine();
void parseMessage();
void parseMessage(byte*, int);
void sendMessage(char, char, const char *); // send c-style string
void sendMessage(char, char, int); // send a number
void sendMessage(byte (*) (byte*));
void sendLayout();
void sendKeyBinding();
void sendKeyBinding(Command *cmd);
byte keypadToByteArr(byte *pDest, boolean fillString=false, boolean nullSeparation=true); // change to return int
int keypadFromByteArray(byte pSource[]);
void byteArrToEeprom(byte [], int, int);
void byteArrFromEeprom(byte pDest[], int startAddr, int length);
void setBindings(const char *);
void setBindings(byte*, int);
void setBindings(byte*);
// boolean commandFromByteArray(Command*&, byte[]);
int commandFromByteArray(Command*&, byte[]);

// ----------------------------------------------------------------------------
// Debug Information
//
boolean debugSerialIn = false;
boolean debugSerialOut = false;
const char opCodeError[] PROGMEM = "opCodeError";
const char headerError[] PROGMEM = "headerError";
const char unknownError[] PROGMEM = "unknownError";
const char lengthError[] PROGMEM = "lengthError";
const char contentError[] PROGMEM = "contentError";
const char commandTypeError[] PROGMEM = "commandTypeError";
const char waitingForApp[] PROGMEM = "Waiting for app";
const char arduinoReady[] PROGMEM = "Arduino Ready";//"AMK ready";
const char appIsClosing[] PROGMEM = "App is closing";

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

void cloneArray();
void rotateMatrix( accessfn afn);
void initTempArray();
void printArray();
void checkOrientation();

// ----------------------------------------------------------------------------
// Initial setup of the encoder, buttons, led, and communication.
// This is a one-time pre-run of stuff we need to get the code running as planned
//
void setup()
{
  int a=0;
  Serial.begin(SERIAL_BAUDRATE);
  nBSerial.debugToPC("Serial.begin()");
  delay(10000);
  // NonBlockingSerial::initSerial(BUFFER_SIZE, SERIAL_BAUDRATE);

  // print device data

  if(debugEEPROM)
  {
    //reservedMemory
    snprintf(message, sizeof(message), "%s%s%s%u%s%u", "firstBoot: ", (EEPROM.read(0)==255) ? "true" : "false", " usesMemory: ", EEPROM.read(1), "activeSet: ", EEPROM.read(2));
    Serial.println(message);

    //commandMemory
    Serial.println("commandMemory: ");
    int startAddr = reservedMemory;
    for(int i=0; i < int(commandMemory); i++)
    {
      printEepromAt(startAddr+i);
      Serial.println(", ");
    }
    Serial.println();
    // int w = 0;
  }

  if(useMemory) //useEEPROM & useEeprom made arduino unstable for some reason
  {
    //entering this block crashes arduino
    Serial.println("Retrieveing keypad");
    byte temp[BUFFER_SIZE];
    byteArrFromEeprom(temp, (activeSet)*(commandMemory/NUM_SETS) + reservedMemory, 58); //maybe recursive function that constructes the keypad?
    int length = keypadToByteArr(temp);
    nBSerial.sendMessage('g', 'b', temp, length);
  }
  else
  {
    Serial.println("Using default keypad");
    for(int i=0; i<NUM_SETS;i++)
    {
      for(int j=0; j<NUM_ROWS*NUM_COLS; j++)
      {
        keypadSet[i][j] = new Chr(KEY_F13);
      }
    }
  }
  Serial.println("Arduino Ready");
  // printArray(); //print keypad

  // pinMode(LED_BUILTIN_TX, INPUT);               // this is a trick to turn off the tx/rx onboard leds
  //pinMode(LED_BUILTIN_RX, INPUT);

//initialize keypad buttons
  for (int i = 0; i < NUM_ROWS*NUM_COLS ; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
    digitalWrite(buttonPins[i], HIGH); // because of pullup resistor
  }

  initTempArray();
  buffer[0] = 0;

// Encoder features:
//  Comment/delete any line to disable the feature, or adjust values in ClickEncoder.h
  encoder.setAccelerationEnabled(true);
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
  // NonBlockingSerial::getSerialData();
  // if(NonBlockingSerial::allReceived)
  //   parseMessage(NonBlockingSerial::dataRecvd, &NonBlockingSerial::dataSendCount);
  nBSerial.getSerialData();
  // nBSerial.echoToPC();
  nBSerial.processData(parseMessage);

  scanPad();
  scanEncoder();
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
        if(appIsOpen) //NTS: it maybe better to just treat the registered keystrokes in the app itself?
          // sendMessage('p', 'k', i); // send the index of the button that was pressed, blocks the key action.
          // char arr[]={'p', 'k', (char)i};
          // nBSerial.debugToPC("pk"+char(i));
          // Serial.println("Pitz and Danzi 4 years +2 days anniversary!!");
          keypadSet[activeSet][i]->Send();
        else
          // Keyboard.press(keypadSets[activeSet][i]); // send the button's action
          keypadSet[activeSet][i]->Send();
      }
      else // button was released
        // Keyboard.release(keypadSets[activeSet][i]); // release the button
        keypadSet[activeSet][i]->Release();
    }
    prevButtonState[i] = buttonState[i]; // this remebers the button's current state, so we can compare to it on the next round of the loop.
  } 

  //delay(1); // a small delay after reading the buttons helps prevent accidental double-presses.
}

void scanEncoder()
{
  static int16_t lastValue{0};
  int16_t value = encoder.getAccumulate();
  if (value != lastValue) // Encoder Rotation
  {
    if(lastValue>value)// clockwise
    { 
      Consumer.write(MEDIA_VOLUME_UP);
      // Keyboard.releaseAll();
      // encoderSet[activeSet][0]->Send();
      // encoderSet[activeSet][0]->Release();
    }
    else// counter-clockwise rotation
    { 
      Consumer.write(MEDIA_VOLUME_DOWN);
      // Keyboard.releaseAll();
      // encoderSet[activeSet][1]->Send();
      // encoderSet[activeSet][1]->Release();
    }
  }
  lastValue = value;
  
  switch (encoder.getButton()) // Encoder Clicks
  {
  case Button::Clicked: // single click
    // Keyboard.press(encoderSets[activeSet][2]);
    Keyboard.releaseAll();
    // encoderSet[activeSet][2]->Send();
    // encoderSet[activeSet][2]->Release();
    break;
  case Button::DoubleClicked: // double click
    currentOrientation = Toggle(currentOrientation, 4);
    checkOrientation();
    sendMessage('s', 'o', currentOrientation);
    break;
  case Button::Held:
    encoderHeld = true;// see Released.
    // if (appIsOpen)
    //     Serial.println("key set: " + String(activeSet));
    break;
  case Button::LongPressRepeat: // right now, getting to long press repeat also triggers held command
    encoderHeld = false; // to prevent trigerring both held & longpressrepeat at the same time.
    // encoderSet[activeSet][4]->Send();
    // encoderSet[activeSet][4]->Release();
    break;
  case Button::Released:
    if (!appIsOpen && encoderHeld) //this happens on encoder hold
    {
      activeSet = Toggle(activeSet, NUM_SETS);
      // statusLed.sequence(3, 150);
    }
    encoderHeld = false;
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

// ----------------------------------------------------------------------------
// Communication Functions
//

//Start app communication
void waitForApp()
{
  // if the right message was recieved, signal ready
  if(opCode == 'a' && header == 'o')
  {
    appIsOpen = true;
    sendMessage('p', 'r', arduinoReady);
  }
  else
    sendMessage('p', 'm', waitingForApp); 
}

// void sendMessage(byte (*procFunction) (byte *pSendBuffer)) // alternative to processData() to accept custom processing function
// {
//     // processes the data that is in dataRecvd[]
//   NonBlockingSerial::checkInit();
//   if (NonBlockingSerial::allReceived) {
  
//     NonBlockingSerial::debugToPC("processData()");
//     NonBlockingSerial::dataSendCount = procFunction(NonBlockingSerial::dataSend);

//     NonBlockingSerial::dataToPC();
//     NonBlockingSerial::allReceived = false; 
//   }
// }

void parseMessage(byte *pRecvBuffer, int recvLength)
{
  // process message and choose corresponding action
  // nBSerial.debugToPC(__func__);

  byte opCode = *pRecvBuffer++;
  byte header = *pRecvBuffer++;
  byte message[BUFFER_SIZE];
  int length;

  if(!appIsOpen)
    waitForApp();
  else
  {
    switch(opCode)
    {
      // app start/stop
      case 'a':
        if(header == 'o')
         nBSerial.debugToPC("Arduino Ready");
        else
          if(header == 'c')
          {
            sendMessage('p', opCode, appIsClosing);
            appIsOpen = false;
          }
        break;

      // get data from arduino
      case 'g': 
        switch(header)
        {
          case 'e':
          {
            Serial.print("commandMemory: ");
            int startAddr = reservedMemory;
            for(int i=0; i < int(commandMemory); i++)
            {
              printEepromAt(startAddr+i);
              Serial.println(", ");
            }
            Serial.println();
          }
            break;
          case 'b':
            length = keypadToByteArr(message, false, true);
            // message[length] = 0;
            // nBSerial.debugToPC((char*)message);
            nBSerial.sendMessage('g', 'b', message, length);
            break;
          case 'c':
            sendLayout();
            break;
          case 'o':
            sendMessage('s', header, currentOrientation);
            // keypadToByteArr();
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
        break;//end get

      // set data on arduino
      case 's':
        switch(header)
        {
          case 'b':
            setBindings(pRecvBuffer);
            length = keypadToByteArr(message);

            if(useMemory)
              byteArrToEeprom(message, reservedMemory + (activeSet)*(commandMemory/NUM_SETS), length);
              // nBSerial.debugToPC("Keypad size ok");

            nBSerial.sendMessage('s', 'b', message, length);// confirmation message
            // message[length] = 0;
            // nBSerial.debugToPC((char*)message);
            break;
          case 'c':
            sendMessage('e', header, headerError);
            break;
          case 'o':
            currentOrientation = atoi(content);
            checkOrientation();
            sendMessage('s', header, currentOrientation);
            //keypadToByteArr();
            break;
          default:
            header = (header < '0') ? 'u' : header; // maybe unnecessary?
            sendMessage('e', header, headerError);
        }
        break;  //end set
      break; // end processing requests
      default:
        sendMessage('e', header, opCodeError);
    }
  }
}

// void loop()
// {
//   if (Serial.available() > 0) // Check for messages from the app
//     {
//       if(appIsOpen)
//           readLine();
//       else
//         waitForApp();
//     }
//     else // otherwise, keep scanning the buttons
//     {
//       scanPad();
//       scanEncoder();
//       checkOrientation();
//       //executeActionFromQueue();
//   }
// }

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

// void sendKeyBinding() //message format: 
// { 
//   int buttonsPerSet = NUM_COLS*NUM_ROWS;
//   for (int k = 0; k < NUM_SETS; k++)
//   {
//     for(int i=0; i<NUM_COLS*NUM_ROWS; i++)
//     {
//       content[(k*buttonsPerSet) + i] = keypadSets[k][i];
//     }
//   }
//   sendMessage('s', 'b', content);
// }

// void sendKeyBinding() //message format: <(key_type,key_value),....>
// { //NTS: not going to work as is for variable type sizes. maybe send command one by one? <set_index, command_index, command_type, command_value>
//   content[0]=0; // 'empty' content
//   char temp[2]; // NTS: problematic with CStr
//   for (int k = 0; k < NUM_SETS; k++)
//   {
//     for(int i=0; i<NUM_COLS*NUM_ROWS; i++)
//     {
//       keypadSet[k][i]->toCString(temp);
//       strcat(content, temp);
//     }
//   }
//   sendMessage('s', 'b', content);
// }

byte keypadToByteArr(byte *pDest, boolean fillString, boolean nullSeparation) //message format: <(key_type,key_value),....>
{ //NTS: should work for variable type sizes?.
  // nBSerial.debugToPC(__func__);

  byte temp[MAX_COMMAND_SIZE];
  byte commandSize;
  byte padding;
  int startAddr = int(pDest);

  for (byte k=0; k < NUM_SETS; k++)
  {
    for(byte i=0; i < NUM_COLS*NUM_ROWS; i++)
    {
      //command to byte array
      commandSize = keypadSet[k][i]->toByteArray(temp);

      //append command to dest/buffer
      for(int n=0; n<commandSize; n++)
      {
        *pDest++ = temp[n];
      }

      //add padding if needed
      padding = 0;
      if(fillString)
        padding += MAX_COMMAND_SIZE-commandSize;
      else
        padding += (nullSeparation && !(keypadSet[k][i]->nullTerm())) ? 1 : 0;

      while(padding-- > 0)
      {
        *pDest++ = 0;
      }
    }
  }
  // Serial.println(int(pDest)-startAddr);
  return int(pDest)-startAddr; // return array length
}

// void sendKeyBinding(Command *cmd) //message format: 
// { 
//   content[0]=0; // 'empty' content
//   char temp[sizeof(cmd)+1]; // NTS: not sure what size it gives
//   cmd->toCString(temp);
//   strcat(content, temp);
//   sendMessage('p', 'q', content);
// }

// sets the AMK's keybindings to those conatined in content (c-string).
// void setBindings(const char *content)
// {
//   int buttonsPerSet = NUM_COLS*NUM_ROWS;
//   for (int k = 0; k < NUM_SETS; k++)
//   {
//     for(int i=0; i<buttonsPerSet; i++)
//     {
//       keypadSets[k][i] = KeyboardKeycode(content[(k*buttonsPerSet) + i]);
//     }
//   }
// }

// void setBindings(byte *pSource, int length)//, functionName) // to know who is usig this atm: eeprom or serial
// {
//   byte temp[MAX_COMMAND_SIZE];
//   int count=0;
//   Command *newCommand;
//   int buttonsPerSet = NUM_COLS*NUM_ROWS;
//   char st[32];
//   for (int k = 0; k < NUM_SETS; k++)
//   {
//     for(int i=0; i<buttonsPerSet; i++)
//     {
//       // // NonBlockingSerial::debugToPC(int(newCommand));// memory adress
//       // // NonBlockingSerial::debugToPC(*newCommand);// value
//       switch(*pSource)
//       {
//         case 'k':
//         {
//           count++;
//           pSource++;
//           Command::Replace(keypadSet[k][i], new Chr(*pSource));
//           break;
//         }
//         case 'c':
//         {
//           count++;
//           pSource++;
//           Command::Replace(keypadSet[k][i], new Cons(*pSource));
//           break;
//         }
//         case 's':
//         {
//           count++;
//           pSource++;
//           while((*pSource)!=0)
//           {
//             temp[count] = *pSource;
//             pSource++;
//             count++;
//           }
//           temp[count] = *pSource;
//           newCommand = new CStr((char*)temp); //commandfrombytearray()
//           Command::Replace(keypadSet[k][i], newCommand);
//           break;
//         }
//         default:
//         {
//           // nBSerial.debugToPC(commandTypeError);
//           sprintf(st, "%s%u%s%d%s%d", "commandTypeError: ", *pSource, "count: ", count, " keynum: ", (k*buttonsPerSet)+i);
//           nBSerial.debugToPC(st);
//           break;
//         }
//       }

//       do
//       {
//         pSource++;
//         count++;
//       } while((*pSource)!=0);
//     }
//   }
// }

void setBindings(byte *pSource)
{
  byte temp[MAX_COMMAND_SIZE];
  byte *nextNull;
  Command *newCommand;
  // int count=0; //for data validation? make sure the full array is used?

  //for each key...
  for (int k = 0; k < NUM_SETS; k++)
  {
    for(int i=0; i<NUM_ROWS*NUM_COLS; i++)
    {
      nextNull = (byte*) memchr(pSource, 0, MAX_COMMAND_SIZE); // read data untill null(incl.)
      memcpy(temp, pSource, (nextNull-pSource)); //insert command data into temp array
      temp[(nextNull-pSource)] = 0;
      if(commandFromByteArray(newCommand, temp)) // make temp array into command
        Command::Replace(keypadSet[k][i], newCommand); // replace command in key // make sure it doesn't destroy the wrong object
      pSource = nextNull+1;
    }
  }
}

// boolean commandFromByteArray(Command *&dest, byte data[])
// {
//   switch(data[0])
//   {
//     case 'k':
//       dest = new Chr(data[1]);
//       break;
//     case 'c':
//       dest = new Cons(data[1]);
//       break;
//     case 's':
//       data++;
//       dest = new CStr((char*)data);
//       break;
//     default:
//       return false;
//   }
//   return true;
// }

int commandFromByteArray(Command *&dest, byte data[])
{
  switch(data[0])
  {
    case 'k':
      dest = new Chr(data[1]);
      break;
    case 'c':
      dest = new Cons(data[1]);
      break;
    case 's':
      data++;
      dest = new CStr((char*)data);
      break;
    default:
      return 0;
  }
  return strlen((char*)data) + int(dest->nullTerm());
}

int byteArrToKeypad(byte pSource[], Command ***pDest)//, functionName) // to know who is usig this atm: eeprom or serial
{
  byte temp[MAX_COMMAND_SIZE];
  byte *nextNull;
  Command *newCommand;
  int count=0;
  // int count=0; //for data validation? make sure the full array is used?

  //for each key...
  for (int k = 0; k < NUM_SETS; k++)
  {
    for(int i=0; i<NUM_ROWS*NUM_COLS; i++)
    {
      nextNull = (byte*) memchr(pSource, 0, MAX_COMMAND_SIZE); // read data untill null(incl.)
      memcpy(temp, pSource, (nextNull-pSource)); //insert command data into temp array
      temp[(nextNull-pSource)] = 0;
      count += commandFromByteArray(newCommand, temp);
      if(count > 0)
        Command::Replace(pDest[k][i], newCommand); // replace command in key // make sure it doesn't destroy the wrong object
      pSource = nextNull+1;
    }
  }
  return count;
}
// void setBindings(const char *content)
// {
//   int buttonsPerSet = NUM_COLS*NUM_ROWS;
//   for (int k = 0; k < NUM_SETS; k++)
//   {
//     for(int i=0; i<buttonsPerSet; i++)
//     {
//       int index = 2*((k*buttonsPerSet) + i);
//       // char keyType = content[index];
//       // char keyValue = content[index+1];
//       // Serial.println(index);

//       switch(content[index])
//       {
//         case 'k':
//           Command::Replace(keypadSet[k][i], new Chr(byte(content[index+1])));
//           break;
//         case 'c':
//           Command::Replace(keypadSet[k][i], new Cons(byte(content[index+1])));
//           break;
//         // case 's':
//         default:
//           // sendMessage('e', 'b', commandTypeError);
//           Serial.print(commandTypeError);
//           Serial.println(" ");
//           Serial.println(content[index]);
//           Serial.println(" ");
//           Serial.println(index);
//         break;
//       }
//     }
//   }
// }

// ----------------------------------------------------------------------------
// Keypad Rotation
//
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

void checkOrientation() // NTS: maybe make sensor reading every few cycles? so it doesn't just poll all the time.
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
// Memory Functions
//
void printEepromAt(int addr)//reads the specified EEPROM address and prints result to serial monitor
{
  byte value = EEPROM.read(addr);
  Serial.print(addr);
  Serial.print(":");
  Serial.print(value);
}

void saveKeypadToEeprom(Command* keypad[NUM_SETS][NUM_ROWS*NUM_COLS]) // seems mostly fine. add memory check
{
  byte temp[MAX_COMMAND_SIZE];
  byte commandSize;
  byte buttonsPerSet = NUM_COLS*NUM_ROWS;
  int index = reservedMemory;// uint_16?

  if(debugEEPROM)
    Serial.println(__func__); //debugToPc(__func__);

  for (byte k=0; k < NUM_SETS; k++)
  {
    for(byte i=0; i < buttonsPerSet; i++)
    {
      commandSize = keypad[k][i]->toByteArray(temp);// check that temp is actually written to

      // write array to EEPROM
      for(byte n=0; n<MAX_COMMAND_SIZE; n++)
      {
        EEPROM.update(index+n, temp[n]);
      }

      index += MAX_COMMAND_SIZE-commandSize+1; // supposed to add up to MAX_COMMAND_SIZE
      // printEepromAt(address);
    }
  }
}

// void saveKeypadToEeprom(KeyboardKeycode bindings[NUM_SETS][NUM_COLS*NUM_ROWS])
// {
//   byte key;
//   byte buttonsPerSet = NUM_COLS*NUM_ROWS;
//   int keypadStartAddr = reservedMemory;
//   int address;

//   if(debugEEPROM)
//     Serial.println("Saving keypad to EEPROM");

//   for (byte k=0; k < NUM_SETS; k++)
//   {
//     for(byte i=0; i < buttonsPerSet; i++)
//     {
//       address = (k*buttonsPerSet)+i+keypadStartAddr;
//       key =  bindings[k][i];
//       EEPROM.update(address, key);
//       printEepromAt(address);
//     }
//   }
// }
void getKeypadFromEeprom()
{
  byte buttonsPerSet = NUM_COLS*NUM_ROWS;
  int keypadStartAddr = reservedMemory;
  int address;

  for (byte k=0; k < NUM_SETS; k++)
  {
    for(byte i=0; i < buttonsPerSet; i++)
    {
      address = (k*buttonsPerSet)+i+keypadStartAddr;
      keypadSets[k][i] = KeyboardKeycode(EEPROM.read(address));
      // Serial.println(keypadSets[k][i]);
    }
  }
}

void byteArrFromEeprom(byte pDest[], int startAddr, int length)
{
  for (int i=0; i < length; i++)
    *pDest++ = EEPROM.read(startAddr+i);
}

void byteArrToEeprom(byte source[], int startAddr, int length)
{
  if(length <= int(sizePerSet))
    for(int i=0; i<length; i++)
    {
      EEPROM.update(startAddr+i, source[i]);
    }
  else
    nBSerial.debugToPC("Source exceeds allowed size");
}

int keypadStartAdress(int setIndex)
{
  int count=0;
  for(int i=0; i<=int(commandMemory)-1; i++)
  {
    if(count==setIndex)
      return reservedMemory+i;
    else if(EEPROM[reservedMemory+i]==0 && EEPROM[reservedMemory+i+1]==0)
      count++;
  }
  return -1;
}

int keypadMemSize(int setIndex)
{
  int startAddr = keypadStartAdress(setIndex), addr=startAddr;
  int count=0;

  if(startAddr < 0)
    return 0;

  while(count < (NUM_ROWS*NUM_ROWS)) // find the #(buttonsPerSet) terminator
  {
    count+= (EEPROM.read(addr)==0) ? 1 : 0;
    addr++;
  }
  return addr-startAddr;
  // for(int i=0; i<sizePerSet; i++)
  // {
  //   count+= (EEPROM.read(startAddr+i)==0) ? 1 : 0;
  //   if(count==(NUM_ROWS*NUM_COLS))
  //     addr = startAddr+i;
  // }
  // return addr;
}
// void getKeypadTFromEeprom()
// {
//   byte buttonsPerSet = NUM_COLS*NUM_ROWS;
//   int keypadStartAddr = reservedMemory;
//   int index;

//   for (byte k=0; k < NUM_SETS; k++)
//   {
//     for(byte i=0; i < buttonsPerSet; i++)
//     {
//       index = (k*buttonsPerSet)+i+keypadStartAddr;
//       keypadSets[k][i] = KeyboardKeycode(EEPROM.read(index));
//       // Serial.println(keypadSets[k][i]);
//     }
//   }
// }

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
