/*
  RawByteSerial.h - Library for serial communication using raw Bytes.
  Created by .
  Released into the public domain.
*/
#ifndef RawByteSerial
#define RawByteSerial

#include "Arduino.h"

// const byte startMarker;// = 254;
// const byte endMarker;// = 255;
// const byte specialByte;// = 253;
//extern const byte maxMessage;// = 16;

// byte bytesRecvd;
// byte dataSentNum; // the transmitted value of the number of bytes in the package i.e. the 2nd byte received
// byte dataRecvCount;

// extern byte *dataRecvd;
// extern byte *dataSend;  
// extern byte *tempBuffer;

void init(byte maxMessage);
void getSerialData();
void decodeHighBytes();
void processData();
void dataToPC();
void encodeHighBytes();
void debugToPC( char arr[]);
void debugToPC( byte num);
void blinkLED(byte numBlinks);

// class Buffers 
//   {
//   public:
//     //constructor and deconstructor
//     Buffers(uint8_t len);
//     ~Buffers();

//     //variables
//     uint8_t len_;
//     byte* dataRecvd;
//     byte* dataSend;
//     byte* tempBuffer;
//   };    // end of class bufr

// class Convsersation
// {
//   public:
//     Convsersation(int pin);
//     void getSerialData();
//     void decodeHighBytes();
//     void processData();
//     void dataToPC();
//     void encodeHighBytes();
//     void debugToPC( char arr[]);
//     void debugToPC( byte num);
//     void blinkLED(byte numBlinks);
//   private:
//     const byte startMarker;// = 254;
//     const byte endMarker;// = 255;
//     const byte specialByte;// = 253;
//     const byte maxMessage;// = 16;

//     byte bytesRecvd = 0;
//     byte dataSentNum = 0; // the transmitted value of the number of bytes in the package i.e. the 2nd byte received
//     byte dataRecvCount = 0;


//     byte dataRecvd[maxMessage]; 
//     byte dataSend[maxMessage];  
//     byte tempBuffer[maxMessage];

//     byte dataSendCount = 0; // the number of 'real' bytes to be sent to the PC
//     byte dataTotalSend = 0; // the number of bytes to send to PC taking account of encoded bytes

//     boolean inProgress = false;
//     boolean startFound = false;
//     boolean allReceived = false;
// };

#endif