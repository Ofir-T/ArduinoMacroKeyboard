/*Non-Blocking Serial. (Ofir Temelman. August 8 2022)
  This class is adapted from Robin2's demo of serial communication posted
  on the arduino forum on 12 Mar 2014: https://forum.arduino.cc/t/demo-of-pc-arduino-comms-using-python/219184.
  This works with ComArduino.py and ComArduinoA4e.rb
  this version uses a start marker 254 and an end marker of 255
  it uses 253 as a special byte to be able to reproduce 253, 254 and 255
  it also sends data to the PC using the same system
  if the number of bytes is 0 the PC will assume a debug string and just print it to the screen
*/
#include <Arduino.h>
#include <NonBlockingSerial.h>
//================

// the program could be rewritten to use local variables instead of some of these globals
//  however globals make the code simpler
//  and simplify memory management

// const byte startMarker = byte('{');//254;
// const byte endMarker = byte('}');//255;
// const byte specialByte = byte('z');//253;
// const byte maxMessage = 65;

// byte bytesRecvd = 0;
// byte dataSentNum = 0; // the transmitted value of the number of bytes in the package i.e. the 2nd byte received
// byte dataRecvCount = 0;

// byte *dataRecvd;//maxMessage]; 
// byte *dataSend;//maxMessage];  
// byte *tempBuffer;//maxMessage];

// byte dataSendCount = 0; // the number of 'real' bytes to be sent to the PC
// byte dataTotalSend = 0; // the number of bytes to send to PC taking account of encoded bytes

// boolean inProgress = false;
// boolean allReceived = true;
// boolean serialInit = false;

//================
// NonBlockingSerial& NonBlockingSerial::initSerial(const int _maxMessage,const long baudRate)
// { // returns singleton instance of class
//     static NonBlockingSerial    instance; // Guaranteed to be destroyed.
//                           // Instantiated on first use.
//     return instance;
// }
/*
void NonBlockingSerial::initSerial(const int _maxMessage,const long baudRate)//, byte _startMarker, byte _endMarker, byte _specialByte) 
{

  if(!serialInit)
  {
    maxMessage = _maxMessage;
    // bytesRecvd = 0;
    // dataSentNum = 0; // the transmitted value of the number of bytes in the package i.e. the 2nd byte received
    // dataRecvCount = 0;

    *&dataRecvd = new byte[maxMessage]; 
    *&dataSend = new byte[maxMessage];  
    *&tempBuffer = new byte[maxMessage];

    // dataSendCount = 0;
    // dataTotalSend = 0;

    // inProgress = false;
    // allReceived = true; //NTS:set to false!!
    // serialInit = false;

    Serial.begin(baudRate);
    serialInit = true;
    debugToPC("Serial initialized");
  }
  else
  {
    debugToPC("Serial already initialized!");
  }
}

void NonBlockingSerial::checkInit()
{
  if(serialInit == false)
  {
    debugToPC("not init!");
  }
}

//============================

void NonBlockingSerial::getSerialData() {

     // Receives data into tempBuffer[]
     //   saves the number of bytes that the PC said it sent - which will be in tempBuffer[1]
     //   uses decodeHighBytes() to copy data from tempBuffer to dataRecvd[]
     
     // the Arduino program will use the data it finds in dataRecvd[]
  checkInit();
  if(Serial.available() > 0)
  {

    byte x = Serial.read();
    if (x == startMarker)
    { 
      bytesRecvd = 0; 
      inProgress = true;
      // blinkLED(2);
      debugToPC("start received");
    }
      
    if(inProgress)
    {
      tempBuffer[bytesRecvd] = x;
      bytesRecvd ++;
    }

    if (x == endMarker)
    {
      inProgress = false;
      allReceived = true;
      
        // save the number of bytes that were sent
      dataSentNum = tempBuffer[1];
  
      decodeHighBytes();
    }
  }
}

//============================

void NonBlockingSerial::echoToPC() {

  // processes the data that is in dataRecvd[]
  checkInit();
  if (allReceived) {
    debugToPC("processData()");
      // for demonstration just copy dataRecvd to dataSend
    dataSendCount = dataRecvCount;
    for (byte n = 0; n < dataRecvCount; n++) {
       dataSend[n] = dataRecvd[n];
    }

    dataToPC();

    //delay(100);
    allReceived = false; 
  }
}

//============================

void NonBlockingSerial::decodeHighBytes() {

  //  copies to dataRecvd[] only the data bytes i.e. excluding the marker bytes and the count byte
  //  and converts any bytes of 253 etc into the intended numbers
  //  Note that bytesRecvd is the total of all the bytes including the markers
  checkInit();
  debugToPC("decodeHighBytes()");
  dataRecvCount = 0;
  for (byte n = 2; n < bytesRecvd - 1 ; n++) { // 2 skips the start marker and the count byte, -1 omits the end marker
    byte x = tempBuffer[n];
    if (x == specialByte) {
       // debugToPC("FoundSpecialByte");
       n++;
       x = x + tempBuffer[n];
    }
    dataRecvd[dataRecvCount] = x;
    dataRecvCount ++;
  }
}

//====================

void NonBlockingSerial::dataToPC() {

      // expects to find data in dataSend[]
      //   uses encodeHighBytes() to copy data to tempBuffer
      //   sends data to PC from tempBuffer
    checkInit();
    debugToPC("dataToPC()");
    encodeHighBytes();

    Serial.write(startMarker);
    Serial.write(dataSendCount);
    Serial.write(tempBuffer, dataTotalSend);
    Serial.write(endMarker);
}

//============================

void NonBlockingSerial::encodeHighBytes() {
  // Copies to temBuffer[] all of the data in dataSend[]
  //  and converts any bytes of 253 or more into a pair of bytes, 253 0, 253 1 or 253 2 as appropriate
  checkInit();
  debugToPC("encodeHighBytes()");
  dataTotalSend = 0;
  for (byte n = 0; n < dataSendCount; n++) {
    if (dataSend[n] >= specialByte) {
      tempBuffer[dataTotalSend] = specialByte;
      dataTotalSend++;
      tempBuffer[dataTotalSend] = dataSend[n] - specialByte;
    }
    else {
      tempBuffer[dataTotalSend] = dataSend[n];
    }
    dataTotalSend++;
  }
}

//=========================

void NonBlockingSerial::debugToPC( char arr[]) {
    byte nb = 0; //nb in this case is the length of the message. 0 means debug.
    Serial.write(startMarker);
    Serial.write(nb);
    Serial.print(arr);
    Serial.write(endMarker);
}

//=========================

void NonBlockingSerial::debugToPC( byte num) {
    byte nb = 0;
    Serial.write(startMarker);
    Serial.write(nb);
    Serial.print(num);
    Serial.write(endMarker);
}

//=========================

void NonBlockingSerial::blinkLED(byte numBlinks) {
    for (byte n = 0; n < numBlinks; n ++) {
      digitalWrite(13, HIGH);
      delay(200);
      digitalWrite(13, LOW);
      delay(200);
    }
}

//============================

//rename to prepareToSend
void NonBlockingSerial::processData(void (*procFunction) (byte *pSendBuffer, byte *pSendCound)) // overload of process data to accept custom processing function
{
    // processes the data that is in dataRecvd[]
  checkInit();
  if (allReceived) {
  
    debugToPC("processData()");
    procFunction(dataSend, &dataSendCount);

    dataToPC();
    allReceived = false; 
  }
}

// void NonBlockingSerial::sendMessage(byte (*procFunction) (byte *pSendBuffer)) // overload of process data to accept custom processing function
// {
//     // processes the data that is in dataRecvd[]
//   checkInit();
//   if (allReceived) {
  
//     debugToPC("sendMessage()");
//     dataSendCount = procFunction(dataSend);

//     dataToPC();
//     allReceived = false; 
//   }
// }
//================

// void setup() {
//   pinMode(13, OUTPUT); // the onboard LED
//   Serial.begin(57600);
//   debugToPC("Arduino Ready from ArduinoPC.ino");
  
//   delay(500);
//   blinkLED(5); // just so we know it's alive
// }

//================

// void loop() {

//   getSerialData();
  
//   processData();

// }

//*/
//============================

void NonBlockingSerial2::printMemAddr()
{
  // Serial.println(maxMessage);
  Serial.println(int(dataRecvd));
  Serial.println(int(dataSend));
  Serial.println(int(tempBuffer));
}

void NonBlockingSerial2::getSerialData()
{
  // Receives data into tempBuffer[]
  //   saves the number of bytes that the PC said it sent - which will be in tempBuffer[1]
  //   uses decodeHighBytes() to copy data from tempBuffer to dataRecvd[]
  //   the Arduino program will use the data it finds in dataRecvd[]

  if(Serial.available() > 0)
  {

    byte x = Serial.read();
    if (x == startMarker)
    { 
      bytesRecvd = 0; 
      inProgress = true;
      // blinkLED(2);
      // debugToPC(__func__);
    }
      
    if(inProgress)
    {
      tempBuffer[bytesRecvd] = x;
      bytesRecvd ++;
    }

    if (x == endMarker)
    {
      inProgress = false;
      allReceived = true;
      
        // save the number of bytes that were sent
      dataSentNum = tempBuffer[1];
  
      decodeHighBytes();
    }
  }
}

//============================

void NonBlockingSerial2::echoToPC() {

  // processes the data that is in dataRecvd[]
  if (allReceived) {
    // debugToPC(__func__);
    // for demonstration just copy dataRecvd to dataSend
    dataSendCount = dataRecvCount;
    for (byte n = 0; n < dataRecvCount; n++) {
       dataSend[n] = dataRecvd[n];
    }

    dataToPC();

    //delay(100);
    allReceived = false; 
  }
}

//============================

void NonBlockingSerial2::decodeHighBytes()
{
  //  copies to dataRecvd[] only the data bytes i.e. excluding the marker bytes and the count byte
  //  and converts any bytes of 253 etc into the intended numbers
  //  Note that bytesRecvd is the total of all the bytes including the markers

  // debugToPC(__func__);
  dataRecvCount = 0;
  for (byte n = 2; n < bytesRecvd - 1 ; n++) { // 2 skips the start marker and the count byte, -1 omits the end marker
    byte x = tempBuffer[n];
    if (x == specialByte) {
       // debugToPC("FoundSpecialByte");
       n++;
       x = x + tempBuffer[n];
    }
    dataRecvd[dataRecvCount] = x;
    dataRecvCount ++;
  }
}

//====================

void NonBlockingSerial2::dataToPC()
{
    // expects to find data in dataSend[]
    //   uses encodeHighBytes() to copy data to tempBuffer
    //   sends data to PC from tempBuffer

    // debugToPC(__func__);
    encodeHighBytes();

    Serial.write(startMarker);
    Serial.write(dataSendCount);
    Serial.write(tempBuffer, dataTotalSend);
    Serial.write(endMarker);
}

//============================

void NonBlockingSerial2::encodeHighBytes()
{
  // Copies to temBuffer[] all of the data in dataSend[]
  //  and converts any bytes of 253 or more into a pair of bytes, 253 0, 253 1 or 253 2 as appropriate
  // debugToPC(__func__);
  dataTotalSend = 0;
  for (byte n = 0; n < dataSendCount; n++) {
    if (dataSend[n] >= specialByte) {
      tempBuffer[dataTotalSend] = specialByte;
      dataTotalSend++;
      tempBuffer[dataTotalSend] = dataSend[n] - specialByte;
    }
    else {
      tempBuffer[dataTotalSend] = dataSend[n];
    }
    dataTotalSend++;
  }
}

//=========================

void NonBlockingSerial2::debugToPC( char arr[])
{
    byte nb = 0; //nb in this case is the length of the message. 0 means debug.
    Serial.write(startMarker);
    Serial.write(nb);
    Serial.print(arr);
    Serial.write(endMarker);
}

//=========================

// void NonBlockingSerial2::debugToPC( byte arr[])
// {
//     byte nb = 0; //nb in this case is the length of the message. 0 means debug.
//     Serial.write(startMarker);
//     Serial.write(nb);
//     Serial.write(arr);
//     Serial.write(endMarker);
// }

//=========================

void NonBlockingSerial2::debugToPC(const char arr[])
{
    byte nb = 0; //nb in this case is the length of the message. 0 means debug.
    Serial.write(startMarker);
    Serial.write(nb);
    Serial.print(arr);
    Serial.write(endMarker);
}

//=========================

void NonBlockingSerial2::debugToPC( byte num)
{
    byte nb = 0;
    Serial.write(startMarker);
    Serial.write(nb);
    Serial.print(num);
    Serial.write(endMarker);
}

//=========================

void NonBlockingSerial2::blinkLED(byte numBlinks)
{
    for (byte n = 0; n < numBlinks; n ++) {
      digitalWrite(13, HIGH);
      delay(200);
      digitalWrite(13, LOW);
      delay(200);
    }
}

//============================

//rename to prepareToSend
void NonBlockingSerial2::processData(void (*procFunction) (byte *pRecvBuffer))//, int recvdCount))
{
    // processes the data that is in dataRecvd[]
  if (allReceived) {
  
    // debugToPC(__func__);
    procFunction(dataRecvd);//, dataRecvCount);

    allReceived = false; 
  }
}

void NonBlockingSerial2::processData(void (*procFunction) (byte *pRecvBuffer, int recvdCount))
{
    // processes the data that is in dataRecvd[]
  if (allReceived) {
  
    // debugToPC(__func__);
    procFunction(dataRecvd, dataRecvCount);

    allReceived = false; 
  }
}

void NonBlockingSerial2::sendMessage(byte (*procFunction) (byte *pSendBuffer))
{
    // processes the data that is in dataRecvd[]
    // debugToPC(__func__); // send function name
  if (allReceived) {
    
    //debugToPC(procFunctionName);
    // byte *arr = new byte[maxMessage];
    dataSendCount = procFunction(dataSend);

    dataToPC();
    allReceived = false; 
  }
}

void NonBlockingSerial2::sendMessage(byte message[], byte length)
{
    // processes the data that is in dataRecvd[]
    // debugToPC(__func__); // send function name
  if (allReceived) {

    dataSendCount = sizeof(message);
    for (byte n = 0; n < length; n++) 
       dataSend[n] = message[n];

    dataToPC();
    allReceived = false; 
  }
}

void NonBlockingSerial2::sendMessage(byte opCode, byte header, byte value[], byte length) // in tlv scheme
{
    // processes the data that is in dataRecvd[]
    // debugToPC(__func__); // send function name
  if (allReceived) {

    //'type'
    dataSend[0] = opCode;
    dataSend[1] = header;

    //'length'
    dataSendCount = 2+length; //to include opCode and Header (the 'type')
    
    //'value'
    for (byte n = 2; n < dataSendCount; n++)
    {
       dataSend[n] = value[n-2];
    }


    dataToPC();
    allReceived = false; 
  }
}