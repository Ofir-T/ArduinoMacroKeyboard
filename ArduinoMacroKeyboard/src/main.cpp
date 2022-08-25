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
// #include <MemoryUsage.h>
// #include "Configuration.h"
#include <ClickEncoder.h> // by Schallbert
#include <TimerOne.h>
#include <HID-Project.h>
#include <Blink.h>
#include <EEPROM.h>
#include <NonBlockingSerial.h>
#include <Command.h>

// ----------------------------------------------------------------------------
// Name. This is the name of your AMK.
//
const char *amkName= "Yo Mama";

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
constexpr int NUM_ROWS = 3;                                    // the size of the button grid: 3x3, 4x2, etc.
constexpr int NUM_COLS = 3;                                    // the size of the button grid: 3x3, 4x2, etc.
constexpr int buttonPins[NUM_ROWS*NUM_COLS] = {   4, 3, 2,     // [1] [2] [3]
                                                  10, 6, 5,    // [4] [5] [6]
                                                  9, 7, 8   }; // [7] [8] [9]
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

Command *keypad[NUM_SETS][NUM_ROWS*NUM_COLS];
Command *tempKeypad[NUM_SETS][NUM_ROWS*NUM_COLS];
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
  [0: firstBoot, 64->1024: Commands]

*/

const uint16_t reservedMemory = 64; // for future use
constexpr uint16_t reservedRam = 1000; // according to rest of the program
constexpr uint16_t keysetMem = reservedRam/4;
constexpr uint16_t commandMemory = 1024-reservedMemory;
const uint16_t sizePerSet = (960/NUM_SETS)-(NUM_ROWS*NUM_COLS)-NUM_ENC_CMD-1; // subtract null terminators
uint16_t availableCommandMemory = 960;
constexpr uint16_t BUFFER_SIZE = keysetMem+5; // +startMarker, endMarker, length, tag(2B)
constexpr byte MAX_COMMAND_SIZE = Command::STRING_LENGTH+2; // 1B type, 1B seperator. Change STRING_LENGTH in Command.h to change this
constexpr uint16_t MAX_COMMANDS = commandMemory/MAX_COMMAND_SIZE; //(useMemory) ? ((EEPROM.length()-reservedMemory)/MAX_STRING_LENGTH):(960/MAX_STRING_LENGTH);

boolean debugEEPROM = false;
boolean useMemory = false; // set to false if you don't want to store data in EEPROM.

// constexpr int numcommands = NUM_SETS*(NUM_ENC_CMD + (NUM_ROWS*NUM_COLS));
// static_assert((numcommands <= MAX_COMMANDS), "Number of commands defined exceeds reserved memory");

int keypadStartAdress(int setIndex);
int keypadMemSize(int setIndex);
void printEepromAt(int addr);
void byteArrToEeprom(byte [], int, int);
void byteArrFromEeprom(byte pDest[], int startAddr, int length=keypadMemSize(activeSet));

// ----------------------------------------------------------------------------
// Serial communication
//
constexpr long SERIAL_BAUDRATE = 56700;  // unnecessary since hid opens serial port anyway?
byte inBuffer[BUFFER_SIZE], outBuffer[BUFFER_SIZE], tmpBuffer[BUFFER_SIZE];
NonBlockingSerial2 nBSerial(BUFFER_SIZE, inBuffer, outBuffer, tmpBuffer);

// ----------------------------------------------------------------------------
// Companion app
//

boolean appIsOpen = false;                    
void parseMessage(byte*, int);
void sendMessage(byte (*) (byte*));
// void sendKeyBinding(Command *cmd);
void keypadFromByteArray(byte*);
int keypadToByteArray(byte *pDest, boolean fillString=false, boolean nullSeparation=true); // change to return int
// int keypadFromByteArray(byte pSource[]);
int commandFromByteArray(Command*&, byte[]);

// ----------------------------------------------------------------------------
// Debug Information
//
// boolean debugSerialIn = false;
// boolean debugSerialOut = false;

// commonly used strings. defining them this way reduces ram useage
const char *opCodeError= "opCodeError";
const char *headerError= "headerError";
const char *unknownError= "unknownError";
const char *lengthError= "lengthError";
const char *contentError= "contentError";
const char *commandTypeError= "commandTypeError";
const char *waitingForApp= "Waiting for app";
const char *arduinoReady= "Arduino Ready";//"AMK ready";
const char *appIsClosing= "App is closing";

// ----------------------------------------------------------------------------
// Keypad rotation
//

byte currentOrientation = 3; // 0,1,2,3 -> top, left, bottom, right
byte lastOrientation = 3;
byte delta = currentOrientation - lastOrientation;

typedef Command*& (*accessfn)(int k, int x, int y);
Command*& normalK( int k, int x, int y){ return keypad[k][(NUM_COLS*x)+y]; }
Command*& rotateCW( int k, int x, int y){ return keypad[k][(NUM_COLS-1-x)+3*y]; }   // <<<<<< This is the main thing
Command*& rotateCCW( int k, int x, int y){ return keypad[k][3*(NUM_COLS-1-y)+x]; }

void copyKeypad(Command *pSource[NUM_SETS][NUM_ROWS*NUM_COLS], Command *pDest[NUM_SETS][NUM_ROWS*NUM_COLS]);
void rotateKeypad( accessfn afn);
void checkOrientation();

// ----------------------------------------------------------------------------
// Initial setup of the encoder, buttons, led, and communication.
// This is a one-time pre-run of stuff we need to get the code running as planned
//
void setup()
{
  Serial.begin(SERIAL_BAUDRATE);
  nBSerial.debugToPC("Serial.begin()");
  delay(10000);
  // NonBlockingSerial::initSerial(BUFFER_SIZE, SERIAL_BAUDRATE);

  // // print device data
  sprintf((char*)tmpBuffer, "%s%s%s%s", ("firstBoot: "), (EEPROM.read(0)==255) ? ("true") : ("false"), (" usesMemory: "), (EEPROM.read(1)==255) ? ("true") : ("false"));
  nBSerial.debugToPC((char*)tmpBuffer);

  if(debugEEPROM)
  {
    //commandMemory
    Serial.println(F("commandMemory: "));
    int startAddr = reservedMemory;
    for(int i=0; i < int(commandMemory); i++)
    {
      printEepromAt(startAddr+i);
      Serial.println(F(", "));
    }
    Serial.println();
  }

  Serial.println(F("Using default keypad"));
  for(int i=0; i<NUM_SETS;i++)
  {
    for(int j=0; j<NUM_ROWS*NUM_COLS; j++)
    {
      keypad[i][j] = new Chr((KEY_A+(i*NUM_ROWS*NUM_COLS)+j));
    }
  }
  
  if(useMemory)
  {
    int length=0;
    if(EEPROM.read(0)==0)
    {
      // insert default keypad to eeprom
      length = keypadToByteArray(tmpBuffer);
      byteArrToEeprom(tmpBuffer, reservedMemory + (activeSet)*(commandMemory/NUM_SETS), length);
    }
    Serial.println(F("Retrieveing keypad"));
    byteArrFromEeprom(tmpBuffer, (activeSet)*(commandMemory/NUM_SETS) + reservedMemory, 59); //maybe recursive function that constructes the keypad?

    // print keypad
    length = keypadToByteArray(tmpBuffer);
    tmpBuffer[length] = 0;
    nBSerial.debugToPC((char*)tmpBuffer);
  }

  // pinMode(LED_BUILTIN_TX, INPUT);               // this is a trick to turn off the tx/rx onboard leds
  //pinMode(LED_BUILTIN_RX, INPUT);

  //initialize arduino pins
  for (int i = 0; i < NUM_ROWS*NUM_COLS ; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
    digitalWrite(buttonPins[i], HIGH); // because of pullup resistor
  }

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

  nBSerial.debugToPC(arduinoReady);
}

// ----------------------------------------------------------------------------
// the main body of our code. this loop runs continuously with our code inside it
//
void loop() 
{
  // NonBlockingSerial::getSerialData();
  // if(NonBlockingSerial::allReceived)
  //   parseMessage(NonBlockingSerial::dataRecvd, &NonBlockingSerial::dataSendCount);
  nBSerial.getSerialData(); // if available
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
  for (int i = 0; i < NUM_ROWS*NUM_COLS ; i++)
  {
    buttonState[i] = digitalRead(buttonPins[i]); // read current button state
    if ((buttonState[i] != prevButtonState[i])) // button changed state
    {
      if(buttonState[i] == LOW) // button was pressed
      {
        if(appIsOpen)
          // sendMessage('p', 'k', i);
          nBSerial.debugToPC("key pressed: " + char(i)); // tell app that a key was pressed. doesn't actuate command.
          //  Serial.println("Pitz and Danzi 4 years +2 days anniversary!!");
        else
          keypad[activeSet][i]->Send(); // actuate command
      }
      else // button was released
        keypad[activeSet][i]->Release();
    }
    prevButtonState[i] = buttonState[i];
  } 

  //delay(1); // a small delay after reading the buttons helps prevent accidental double-presses.
}

void scanEncoder()
{
  static int16_t lastValue{0};
  int16_t value = encoder.getAccumulate();
  if (value != lastValue) // Encoder Rotation
  {
    if(lastValue>value) // clockwise
    { 
      Consumer.write(MEDIA_VOLUME_UP);
      Keyboard.releaseAll();
      // encoderSet[activeSet][0]->Send();
      // encoderSet[activeSet][0]->Release();
    }
    else // counter-clockwise rotation
    { 
      Consumer.write(MEDIA_VOLUME_DOWN);
      Keyboard.releaseAll();
      // encoderSet[activeSet][1]->Send();
      // encoderSet[activeSet][1]->Release();
    }
  }
  lastValue = value;
  
  switch (encoder.getButton()) // Encoder Clicks
  {
    case Button::Clicked: // single click
      Consumer.write(MEDIA_PLAY_PAUSE);
      // encoderSet[activeSet][2]->Send();
      // encoderSet[activeSet][2]->Release();
      break;
    case Button::DoubleClicked: // double click
      currentOrientation = Toggle(currentOrientation, 4);
      checkOrientation();
      nBSerial.debugToPC("orientation changed to " + char(currentOrientation));
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
    /*
      READ IMU
    // readOrientationSensor();
    */
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
  int length;

  if(!appIsOpen)
  {
    if(opCode == 'a' && header == 'o')
    {
      appIsOpen = true;
      nBSerial.debugToPC(arduinoReady);
    }
    else
      nBSerial.debugToPC(waitingForApp);
  }
  else
  {
    // parse message
    switch(opCode)
    {
      // app start/stop
      case 'a':
        switch(header)
        {
          case 'o':
            nBSerial.debugToPC(arduinoReady);
            break;
          case 'c':
            appIsOpen = false;
            nBSerial.debugToPC(appIsClosing);
            break;
          default:
            header = (header < '0') ? 'u' : header;
            nBSerial.debugToPC(headerError);
        }
        break;

      // get data from arduino
      case 'g': 
        switch(header)
        {
          case 'e':
          {
            Serial.println(F("commandMemory: "));
            int startAddr = reservedMemory;
            for(int i=0; i < int(commandMemory); i++)
            {
              printEepromAt(startAddr+i);
              Serial.println(F(", "));
            }
            Serial.println();
          }
            break;
          case 'b':
            length = keypadToByteArray(tmpBuffer, false, true);
            nBSerial.sendMessage('s', 'b', tmpBuffer, length);
            break;
          case 'c':
            sprintf((char*)tmpBuffer, "%d%d%d",NUM_ROWS, NUM_COLS, NUM_SETS);
            nBSerial.sendMessage('s', 'c', tmpBuffer, 3);
            break;
          case 'o':
            tmpBuffer[0] = currentOrientation;
            nBSerial.sendMessage('s', 'o', tmpBuffer, 1);
            // keypadToByteArray();
            break;
          case 'a':
            tmpBuffer[0] = activeSet;
            nBSerial.sendMessage('s', 'a', tmpBuffer, 1);
            break;
          case 'n':
            memcpy(tmpBuffer, amkName, strlen(amkName)-1);
            nBSerial.sendMessage('s', 'n', tmpBuffer, strlen(amkName)-1);
            break;
          default:
            header = (header < '0') ? 'u' : header;
            nBSerial.debugToPC(headerError);
            // sendMessage('e', header, headerError);
        }
        break;//end get

      // set data on arduino
      case 's':
        switch(header)
        {
          case 'b':
            keypadFromByteArray(pRecvBuffer);
            length = keypadToByteArray(tmpBuffer);

            if(useMemory)
              byteArrToEeprom(tmpBuffer, reservedMemory + (activeSet)*(commandMemory/NUM_SETS), length);

            tmpBuffer[length] = 0;
            nBSerial.debugToPC((char*)tmpBuffer); // confirmation message
            break;
          case 'o':
            currentOrientation = *pRecvBuffer;
            checkOrientation();
            tmpBuffer[0] = currentOrientation;
            nBSerial.sendMessage('s', 'o', tmpBuffer, 1);
            //keypadToByteArray();
            break;
          default:
            header = (header < '0') ? 'u' : header; // maybe unnecessary?
            nBSerial.debugToPC(headerError);
            // sendMessage('e', header, headerError);
        }
        break;  //end set
      break; // end processing requests
      default:
        nBSerial.debugToPC(headerError);
        // sendMessage('e', header, opCodeError);
    }
  }
}

int keypadToByteArray(byte *pDest, boolean fillString, boolean nullSeparation) //message format: <(key_type,key_value),....>
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
      commandSize = keypad[k][i]->toByteArray(temp);

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
        padding += (nullSeparation && !(keypad[k][i]->nullTerm())) ? 1 : 0;

      while(padding-- > 0)
      {
        *pDest++ = 0;
      }
    }
  }
  // Serial.println(int(pDest)-startAddr);
  return int(pDest)-startAddr; // return array length
}

// void sendKeyBinding(Command *cmd) // sends a single binding over serial
// { 
//   content[0]=0; // 'empty' content
//   char temp[sizeof(cmd)+1]; // NTS: not sure what size it gives
//   cmd->toCString(temp);
//   strcat(content, temp);
//   sendMessage('p', 'q', content);
// }

void keypadFromByteArray(byte *pSource)
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
        Command::Replace(keypad[k][i], newCommand); // replace command in key // make sure it doesn't destroy the wrong object
      pSource = nextNull+1;
    }
  }
}


int commandFromByteArray(Command *&dest, byte data[]) // creates a Command from and array of bytes
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

int byteArrToKeypad(byte pSource[], Command ***pDest)
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
        Command::Replace(pDest[k][i], newCommand);
      pSource = nextNull+1;
    }
  }
  return count;
}

// ----------------------------------------------------------------------------
// Keypad Rotation
//

void copyKeypad(Command *pSource[NUM_SETS][NUM_ROWS*NUM_COLS], Command *pDest[NUM_SETS][NUM_ROWS*NUM_COLS])
{
  for (int k = 0; k < NUM_SETS; k++)
    for (int i = 0; i < NUM_ROWS*NUM_COLS; i++)
      pDest[k][i] = *&pSource[k][i];
}

void rotateKeypad( accessfn afn)
{
  copyKeypad(keypad, tempKeypad);
  for (int k = 0; k < NUM_SETS; k++)
  {
    for (int x=0; x<NUM_ROWS; x++)
    {
      for(int y=0; y<NUM_COLS; y++)
      {
        tempKeypad[k][(NUM_COLS*x)+y] = afn(k,x,y);
      }
    }
  }
  copyKeypad(tempKeypad, keypad);
}


void checkOrientation()
{ 
  // if not in app mode!!
  delta = currentOrientation - lastOrientation;

  if(delta > 0)
  {
    for(int j=0; j<delta; j++)
      rotateKeypad(rotateCW);
    // Serial.println("Rotating keypad clockwise");
  }
  else if(delta < 0)
  {
    for(int j=0; j>delta; j--)
      rotateKeypad(rotateCCW);
    // Serial.println("Rotating keypad counter-clockwise");
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
  Serial.print(F(":"));
  Serial.print(value);
}

void byteArrFromEeprom(byte pDest[], int startAddr, int length)
{
  // nBSerial.debugToPC(__func__);
  // length = keypadMemSize(activeSet);
  for (int i=0; i < length; i++)
  {
    *pDest = EEPROM.read(startAddr+i);
    pDest++;
  }
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

// //demonstrate keypad roation
  // for(int i=0; i<4; i++)
  // {
  //   for(int j=0; j<NUM_ROWS*NUM_COLS; j++)
  //   {
  //     keypad[0][j]->Send();
  //     delay(1);
  //     keypad[0][j]->Release();
  //     if((j+1)%NUM_COLS==0)
  //       Keyboard.write(KEY_ENTER);
  //     else
  //       Keyboard.write(KEY_SPACE);
  //   }
  //   Keyboard.write(KEY_ENTER);
  //   currentOrientation = Toggle(currentOrientation, 4);
  //   checkOrientation();
  // }

// void saveKeypadToEeprom(Command* keypad[NUM_SETS][NUM_ROWS*NUM_COLS]) // seems mostly fine. add memory check
// {
//   byte temp[MAX_COMMAND_SIZE];
//   byte commandSize;
//   byte buttonsPerSet = NUM_COLS*NUM_ROWS;
//   int index = reservedMemory;// uint_16?

//   if(debugEEPROM)
//     Serial.println(__func__); //debugToPc(__func__);

//   for (byte k=0; k < NUM_SETS; k++)
//   {
//     for(byte i=0; i < buttonsPerSet; i++)
//     {
//       commandSize = keypad[k][i]->toByteArray(temp);// check that temp is actually written to

//       // write array to EEPROM
//       for(byte n=0; n<MAX_COMMAND_SIZE; n++)
//       {
//         EEPROM.update(index+n, temp[n]);
//       }

//       index += MAX_COMMAND_SIZE-commandSize+1; // supposed to add up to MAX_COMMAND_SIZE
//       // printEepromAt(address);
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
