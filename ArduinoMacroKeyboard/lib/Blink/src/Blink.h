/*
  Blink.h - Library for Blinking LED's.
  Created by Ofir Temelman, December 15, 2021.
  Released into the public domain.
*/
#pragma once
#ifndef Blink_h
#define Blink_h

#include "Arduino.h"

class Blink
{
  public:
    Blink(int pin, int durationMs);
    void defaultDuration(int durationMs);
    void sequence(int nBlinks, int duration);
  private:
    int _pin;
    int _durationMs;
};

#endif