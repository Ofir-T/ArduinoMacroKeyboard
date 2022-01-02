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
                executionTime(scanPad);
                executionTime(scanEncoder);
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
}