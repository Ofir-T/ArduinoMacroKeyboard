/*
  Command.h - This library hold the command classes, for storing, sending keyboard actions and macros(sequences of actions).
  Created by Ofir Temelman, August 7, 2022.
*/
#pragma once
#ifndef boardInterface_h
#define boardInterface_h

// //Board Type - uncomment the board type you use
// // #define AVR
// #define BOARD_ESP32C3

#include "Arduino.h"
#include "EEPROM.h"
#include <configuration.h>

#ifdef BOARD_ESP32C3
  #include <BleKeyboard.h>
  static BleKeyboard keyboard("Arduino Macro Keyboard", "TroubledMaker", 100);
  #define KEYBOARD_BEGIN keyboard.begin()
  #define KEYBOARD_PRESS(key) keyboard.press(key)
  #define KEYBOARD_RELEASE(key) keyboard.release(key)
  #define KEYBOARD_WRITE(cstring) keyboard.write((uint8_t*)cstring, strlen(cstring))
  #define CONSUMER_PRESS(key) keyboard.press(key)
  #define CONSUMER_RELEASE(key) keyboard.release(key)
  #define EEPROM_UPDATE(address, data) EEPROM.write(address, data)
  #define IS_CONNECTED keyboard.isConnected()
  #define EEPROM_BEGIN EEPROM.begin(1024)
#else 
  #ifdef BOARD_AVR
    #include <HID-Project.h>
    #define KEYBOARD_BEGIN Keyboard.begin()
    #define KEYBOARD_PRESS(keycode) Keyboard.press(KeyboardKeycode(keycode))
    #define KEYBOARD_RELEASE(keycode) Keyboard.release(KeyboardKeycode(keycode))
    #define KEYBOARD_WRITE(string) Keyboard.print(string)
    #define CONSUMER_PRESS(keycode) Consumer.press(ConsumerKeycode(keycode))
    #define CONSUMER_RELEASE(keycode) Consumer.release(ConsumerKeycode(keycode))
    #define MEMORY_UPDATE(adddress, data) EEPROM.update(address, data)
    #define IS_CONNECTED true
  #endif
#endif

#endif