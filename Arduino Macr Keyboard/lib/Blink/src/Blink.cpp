/*
  Blink.cpp - Library for Blinking LED's.
  Created by Ofir Temelman, December 15, 2021.
  Released into the public domain.
*/
#include "Arduino.h"
#include "Blink.h"

Blink::Blink(int pin, int durationMs = 250)
{
  pinMode(pin, OUTPUT);
  _pin = pin;
  _durationMs = durationMs;
}

void Blink::defaultDuration(int durationMs)
{
  _durationMs = durationMs;
}

void Blink::sequence(int nBlinks, int duration = 0)
{
  if (duration == 0)
    duration = _durationMs;

  if (digitalRead(_pin))
  {
    for (int i=0; i<=nBlinks;  i++)
    {
       digitalWrite(_pin, LOW);
       delay(duration);
       digitalWrite(_pin, HIGH);
       delay(duration);
    }
  }
  else
  {
    for (int i=0; i<=nBlinks;  i++)
    {
       digitalWrite(_pin, HIGH);
       delay(duration);
       digitalWrite(_pin, LOW);
       delay(duration);
    }
  }
}