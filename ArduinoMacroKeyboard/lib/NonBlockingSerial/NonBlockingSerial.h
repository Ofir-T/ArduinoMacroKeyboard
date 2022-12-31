/*
  NonBlockingSerial.h - Library for serial communication using raw Bytes.
  Created by .
*/
// #pragma once
#ifndef NonBlockingSerial_h
#define NonBlockingSerial_h

#include <Arduino.h>
/*
class NonBlockingSerial
{
  private:
    static const byte startMarker = byte('{');//254;
    static const byte endMarker = byte('}');//255;
    static const byte specialByte = byte('z');//253;
    inline static byte maxMessage;

    inline static byte bytesRecvd = 0;
    inline static byte dataSentNum = 0; // the transmitted value of the number of bytes in the package i.e. the 2nd byte received
    inline static byte dataRecvCount = 0;

    // static byte *dataRecvd;//maxMessage]; 
    // static byte *dataSend;//maxMessage];  
    inline static byte *tempBuffer;//maxMessage];

    // static byte dataSendCount; // the number of 'real' bytes to be sent to the PC
    inline static byte dataTotalSend = 0; // the number of bytes to send to PC taking account of encoded bytes

    inline static boolean inProgress = false;
    inline static boolean serialInit = false;

  public:
    inline static byte *dataRecvd;//maxMessage]; //need protections for the pointer vars e.g. dataRecvd - read only, const pointer
    inline static byte *dataSend;//maxMessage];
    inline static byte dataSendCount = 0; // the number of 'real' bytes to be sent to the PC
    inline static boolean allReceived = true; // set to false!

    // static NonBlockingSerial& getInstance(const int _maxMessage, const long baudRate);
    static void initSerial(const int _maxMessage, const long baudRate);
    static void checkInit();
    static void getSerialData();
    static void decodeHighBytes();
    static void echoToPC();
    static void processData(void (*procFunction) (byte *pSendBuffer, byte *pSendCount)); //get rid of this and restore the example, but hide it from interface
    // static void processData(void (*procFunction) (void));
    static void dataToPC();
    static void encodeHighBytes();
    static void debugToPC( char arr[]);
    static void debugToPC( byte num);
    static void blinkLED(byte numBlinks);
    // get & set for all received
    // get & set for all public vars
};//*/

class NonBlockingSerial2
{
  private:
    const byte startMarker = byte('{');//254;
    const byte endMarker = byte('}');//255;
    const byte specialByte = 253;
    const byte maxMessage;

    byte *dataRecvd; 
    byte *dataSend; 
    byte *tempBuffer;

    byte bytesRecvd = 0;
    byte dataSentNum = 0; // the transmitted value of the number of bytes in the package i.e. the 2nd byte received
    byte dataRecvCount = 0;

    byte dataSendCount = 0; // the number of 'real' bytes to be sent to the PC
    byte dataTotalSend = 0; // the number of bytes to send to PC taking account of encoded bytes

    boolean inProgress = false;
    boolean allReceived = false; // default true is ok?

    void encodeHighBytes();
    void decodeHighBytes();
    void dataToPC();

  public:

    // NonBlockingSerial2(const int _maxMessage):
    //   maxMessage(_maxMessage),
    //    dataRecvd(new byte(_maxMessage)),
    //    dataSend(new byte(_maxMessage)),
    //    tempBuffer(new byte(_maxMessage))
    //   {allReceived=true;};
    NonBlockingSerial2(const int _maxMessage, byte* sendBuffer, byte* recvBuffer, byte* tmpBuffer):
      maxMessage(_maxMessage),
       dataRecvd(*&recvBuffer),
       dataSend(*&sendBuffer),
       tempBuffer(*&tmpBuffer){};
    
    void printMemAddr();
    void printletter();
    void getSerialData();
    void echoToPC();
    void processData(void (*procFunction) (byte *pRecvBuffer));
    void processData(void (*procFunction) (byte *pRecvBuffer, int recvdCount));
    void sendMessage(byte (*procFunction) (byte *pSendBuffer));
    void sendMessage(byte[], byte);
    void sendMessage(byte, byte, byte[], byte);
    void sendMessage(char arr[]);
    // void processData(void (*procFunction) (void));
    void debugToPC( char arr[]);
    void debugToPC(const char arr[]);
    void debugToPC( byte num);
    // void debugToPC(byte arr[]);
    int getDataRecvdCount();
    void blinkLED(byte numBlinks);
};

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

// class NonBlockingSerial
// {
//   public:
//     NonBlockingSerial(const int maxMessage, const long baudRate);
//     void getSerialData();
//     void decodeHighBytes();
//     void processData();
//     void dataToPC();
//     void encodeHighBytes();
//     void debugToPC( char arr[]);
//     void debugToPC( byte num);
//     void blinkLED(byte numBlinks);
//   private:
//     static const byte startMarker;// = 254;
//     static const byte endMarker;// = 255;
//     static const byte specialByte;// = 253;
//     static const byte maxMessage;// = 16;

//     byte bytesRecvd = 0;
//     byte dataSentNum = 0; // the transmitted value of the number of bytes in the package i.e. the 2nd byte received
//     byte dataRecvCount = 0;


//     byte *dataRecvd; 
//     byte *dataSend;  
//     byte *tempBuffer;

//     byte dataSendCount = 0; // the number of 'real' bytes to be sent to the PC
//     byte dataTotalSend = 0; // the number of bytes to send to PC taking account of encoded bytes

//     boolean inProgress = false;
//     boolean startFound = false;
//     boolean allReceived = false;
// };

#endif