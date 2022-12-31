/*
  ConfigEditor.h - Interface for editing the macro keyboard's key mapping, key sets, etc.
  Created by Ofir Temelman, December 17, 2021.
*/
#include <Arduino.h>
#include <HID-Project.h>
#include "ConfigEditor.h"

namespace ConfigEditor
{
    void mainMenu(int editorCmd)
    {
        switch (editorCmd) //check if it matches any of the editor mode comannds
        {
            case 'l':
                //executionTime(scanPad);
                //executionTime(scanEncoder);
                break;
                
            case 'i':
                // initialization actions here
                break;

            case 'k':
                // keypad actions here
                break;

            case 'e':
                // encoder actions here
                break;

            default:
                break;
        }

        return;
    }

    void keypadMenu()
    {
        
    }

    // void keypadSetup()
// {
//   //Print:
//   // switch testing sequence initiated, please press the keypad buttons in the following order:
//   // [1] [2] [3]
//   // [4] [5] [6]
//   // [7] [8] [9]

//   Serial.println("Switch testing sequence initiated, please press the keypad buttons in order:");

//   for (int i = 0; i < NUMPAD_ROWS ; i++)
//   {
//     for(int j = 0; j < NUMPAD_COLS; j++)
//     {
//       Serial.print("[" + String((j+1) + (i*NUMPAD_COLS)) + "] ");
//     }
//     Serial.print("\n");
//   }

//   // test pins
//   int newPinsArray[NUMPAD_ROWS*NUMPAD_COLS];
//   for (int button = 0; button < NUMPAD_ROWS * NUMPAD_COLS ; button++)
//   {
//     newPinsArray[button] = findPin();
//     Serial.print("[" + newPinsArray[button] + "] ");
//     if (button % NUMPAD_COLS == 0 )
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
//     for (int i = 0; i < NUMPAD_ROWS * NUMPAD_COLS ; i++) // goes over every button pin we defined
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
}