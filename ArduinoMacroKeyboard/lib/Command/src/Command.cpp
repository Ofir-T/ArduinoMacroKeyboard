/*
  Command.cpp - Command class implementation.
  Created by Ofir Temelman, August 7, 2022.
*/
#include "Arduino.h"
#include "Command.h"
#include <HID-Project.h>

// -----------class cpp implementation----------- //why not do all in just the header file?
void Command::Replace(Command *&oldCommand, Command *newCommand)// pointer-reference headace, C++ is weird.
{
  delete oldCommand;
  oldCommand = *&newCommand;
}

Chr::Chr(byte _code)
{
  code = _code;
}

void Chr::Send()
{
  Keyboard.press(KeyboardKeycode(code));
  // if(debug)
  // {
  //   Serial.print("Chr: ");
  //   Serial.println(code);
  // }
}

void Chr::Release()
{
  Keyboard.release(KeyboardKeycode(code));
  // Serial.print("Chr: ");
  // Serial.println(code);
}

byte Chr::toByteArray(byte dest[])
{
  // Serial.println(__func__);
  dest[0] = 'k'; //getType()
  dest[1] = code; //getValue()

  return 2; //array length;
}

char Chr::getType()
{
  return 'k';
}

byte Chr::getValue()
{
  return code;
}

Cons::Cons(byte _code)
{
  code = _code;
}

void Cons::Send()
{
  Consumer.write(ConsumerKeycode(code));
  // if(debug)
  // {
  //   Serial.print("Cons: ");
  //   Serial.println(code);
  // }
}

void Cons::Release()
{
  Consumer.release(ConsumerKeycode(code));
  // Serial.print("Cons: ");
  // Serial.println(code);
}

char Cons::getType()
{
  return 'c';
}

byte Cons::getValue()
{
  return code;
}

byte Cons::toByteArray(byte dest[])
{
  dest[0] = 'c';
  dest[1] = code;

  return 2; //array length;
}

CStr::CStr(char source[]) //with padding
{
  memcpy(cstr, source, strlen(source));
  cstr[strlen(source)]=0;
  
  // Serial.write(cstr, strlen(cstr));
  // Serial.println(strlen(cstr)+1);
}

void CStr::Send()
{
  Keyboard.print(cstr);
  // if(debug)
  // {
  //   Serial.print("CStr: ");
  //   Serial.println(code);
  // }
}

void CStr::Release() // implemented for interface consistency.
{
  return; //there's nothing to release after printing a string.
  // Serial.print("CStr: ");
  // Serial.println(code);
}

char CStr::getType()
{
  return 's';
}

byte CStr::getValue() // returns what?
{
  return 0;
}

byte CStr::toByteArray(byte dest[])
{
  dest[0] = 's';
  memcpy(dest+1, cstr, strlen(cstr));
  dest[strlen(cstr)+1] = 0;
  // Serial.print(strlen(dest)+1);

  return strlen((char*)dest)+1;
}

// boolean commandFromByteArray(Command *&dest, byte data[])
// {
//   switch(data[0])
//   {
//     case 'k':
//       dest = new Chr(data[1]);
//       break;
//     case 'c':
//       dest = new Cons(data[1]);
//       break;
//     case 's':
//       data++;
//       dest = new CStr((char*)data);
//       break;
//     default:
//       return false;
//   }
//   return true;
// }
// -----------stuff i dont want to delete yet-----------
// // at this point, i might as well just create a class
// struct command
// {
//   virtual void sendAction () = 0;
//   virtual void setAction (char input) = 0;
// };

// struct consumer : command
// {
//   ConsumerKeycode action;
//   consumer(ConsumerKeycode c): action(c){};
//   void sendAction () {Keyboard.write(action);};
//   void setAction (char input) {action = ConsumerKeycode(input);};
// };

// struct character : command
// {
//   KeyboardKeycode action;
//   character(KeyboardKeycode k): action(k){};
//   void sendAction () {Keyboard.write(action);};
// };

// struct cString: command // not compatible with the rest of the code as of 1/8/22
// {
//   char* action;
//   cString(char* z): action(z){};
//   // cString(char z): action(&z){};
//   void sendAction () {Serial.println(action);};
// };

// struct integer theInt(3);
// struct character theChar('3');
// struct command *cmdArr[3];
// cmdArr[0] = &theInt;
// cmdArr[1] = &theChar;

// cmdArr[0]->sendAction();
// Serial.println("--------");
// cmdArr[1]->sendAction();

// #############################

// struct action
// {
//   boolean isConsumer;
//   char code;
// };

// void sendAction(struct action *act)
// {
//   if(act->isConsumer)
//     // keyboard.write(ConsumerKeycode(action.code));
//     Serial.println(act->code);
//   else
//     // keyboard.write(KeyboardKeycode(action.code));
//     Serial.println(act->code);
// }

// struct macro // an array/list of actions to be executed sequentially
// {

// };

// struct macro actionQueue[]; //the queue of actions to be executed
// //in loop():
// // struct action action1 = {0,'30'};
// // struct action action2 = {1,'a'};

// // struct action *cmdArr[arrSize];
// // cmdArr[0] = &action2;
// // cmdArr[1] = &action1;
// // Serial.begin(9600);

// // for(int i=0; i<arrSize; i++)
// //   sendAction(cmdArr[i]);

// // Serial.println(sizeof(action1));