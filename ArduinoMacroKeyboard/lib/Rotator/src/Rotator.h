/*
  Blink.h - Library for Blinking LED's.
  Created by Ofir Temelman, December 15, 2021.
  Released into the public domain.
*/
#pragma once
#ifndef Blink_h
#define Blink_h

#include "Arduino.h"

extern const int NUM_ROWS, NUM_COLS;
extern int buttonPins[NUM_ROWS][NUM_COLS];

class Rotator
{
  public:
    Rotator(int dstArray, int len);
    void defaultDuration(int durationMs);
    void sequence(int nBlinks, int duration);
  private:
    int _pin;
    int _durationMs;
};

#endif