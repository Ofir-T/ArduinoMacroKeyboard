/*
  RawByteSerial.h - Library for serial communication using raw Bytes.
  Created by .
  Released into the public domain.
*/
#ifndef RawByteSerial
#define RawByteSerial

#include "Arduino.h"

class Morse
{
  public:
    Morse(int pin);
    void dot();
    void dash();
  private:
    int _pin;
};

#endif