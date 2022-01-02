/*
  ConfigEditor.h - Interface for editing the macro keyboard's key mapping, key sets, etc.
  Created by Ofir Temelman, December 17, 2021.
*/
#ifndef ConfigEditor_h
#define ConfiEditor_h

#include <Arduino.h>
#include <HID-Project.h>

namespace ConfigEditor
{
    void mainMenu(int editorCmd);
    void keypadMenu();
}

#endif

struct Keys
{
    char keyName[20];


}