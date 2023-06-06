/*
  Command.h - This library hold the command classes, for storing, sending keyboard actions and macros(sequences of actions).
  Created by Ofir Temelman, August 7, 2022.
*/
#pragma once
#ifndef Command_h
#define Command_h

#include "Arduino.h"
#include <boardInterface.h>

// class amk:command for internal commands

// void replace(Command *&oldCommand, Command *newCommand)
// {
//   delete oldCommand;
//   oldCommand = *&newCommand; //does it persist out of the function scope?
// }

class Command 
{
  
  public: 
    static const byte STRING_LENGTH=10; // incl. null-terminator?
    //static boolean debug; // try and make it false by default/couple it with main.cpp debug flag.
    // static const byte MAX_STRING_LENGTH = 20;
    virtual void Send () = 0; // =0 forces child classes to override this function
    virtual void Release () = 0;
    //virtual void sendAndRelease () = 0;
    virtual byte toByteArray(byte []) = 0;
    virtual char getType() = 0;
    virtual byte getValue() = 0;
    virtual boolean nullTerm() = 0;
    static void Replace(Command *&oldCommand, Command *newCommand);
};
  
class Chr : virtual public Command
{
  private:
    byte code;
  public:
    Chr(byte _code);//: code(_code){};
    void Send ();// { Keyboard.write(KeyboardKeycode(code)); }
    void Release();
    byte toByteArray(byte []);
    char getType();
    byte getValue();
    boolean nullTerm(){return false;};
};

class Cons : virtual public Command
{
  private:
  byte code;
  public:
  static const boolean NullTerm=false;
  Cons(byte _code);//: code(_code){};
  void Send ();// { Keyboard.write(ConsumerKeycode(code)); }
  void Release();
  char getType();
  byte getValue();
  byte toByteArray(byte []);
  boolean nullTerm(){return false;};
};

class CStr : virtual public Command //maybe char[] instead of just char*
{
  private:
  char cstr[STRING_LENGTH]; // advateages vs pointer: somewhat known memory usage,  memory protection. disadvatages: higher memory usage overall? and length control is taken from main. the memory allocation happens in main anyway, and tracking is already needed, so maybe just use char*.
  public:
  CStr(char []);//: code(_code){};
  void Send ();// { Keyboard.print(code); }
  void Release();
  byte toByteArray(byte []);
  char getType();
  byte getValue();
  boolean nullTerm(){return true;};
};

// function that receives input and creates appropriate command type.
// returns true if successful, false otherwise.
// boolean commandFromByteArray(Command*, byte[]);

// -----------header only implementation-----------

// // void replace(Command *&oldCommand, Command *newCommand)
// // {
// //   delete oldCommand;
// //   oldCommand = *&newCommand; //does it persist out of the function scope?
// // }
// class Command 
// {
//   public: 
//   static const byte MAX_STRING_LENGTH = 20;
//   static char buffer[MAX_STRING_LENGTH+2]; // type + content + null char
//   virtual void Send () = 0;   // pure function
//   // virtual void toString() = 0;
//   static void replace(Command *&oldCommand, Command *newCommand)
//   {
//     delete oldCommand;
//     oldCommand = *&newCommand; //does it persist out of the function scope?
//   }
// };
  
// class Chr : public Command
// {
//   private:
//   byte code;
//   public:
//   Chr(byte _code): code(_code){};
//   // void Send () { Keyboard.write(KeyboardKeycode(code)); }
//   void Send ()
//   {
//     Serial.print("Chr: ");
//     Serial.println(code);
//   }
//   // void toString() 
//   // {
//   //   buffer[0] = 'k';
//   //   buffer[1] = (char)code;
//   //   buffer[2] = NULL;
//   // };
// };

// class Cons : public Command
// {
//   private:
//   byte code;
//   public:
//   Cons(byte _code): code(_code){};
//   // void Send () { Keyboard.write(ConsumerKeycode(code)); }
//   void Send ()
//   {
//     Serial.print("Cons: ");
//     Serial.println(code);
//   }
//   // void toString() 
//   // {
//   //   buffer[0] = 'c';
//   //   buffer[1] = (char)code;
//   //   buffer[2] = NULL;
//   // };
// };

// class CStr : public Command //maybe char[] instead of just char*
// {
//   private:
//   char *code;
//   public:
//   CStr(char *_code): code(_code){};
//   // void Send () { Keyboard.print(code); }
//   void Send ()
//   {
//     Serial.print("CStr: ");
//     Serial.println(code);
//   }
//   // void toString() // incomplete!!
//   // {
//   //   buffer[0] = 's';
//   //   buffer[1] = (char)code; // strcpy or appropriate function
//   // };
// };

#endif

//------------------example main.cpp------------------
// void setup()
// {
//   Serial.begin(9600);
//   Command * objArray [3];
//   Chr *theKey = new Chr('a');
//   Cons *theCons = new Cons('a');
//   CStr *theCStr = new CStr("cstring");
//   objArray[0] = theKey;
//   objArray[1] = theCons;
//   objArray[2] = theCStr;

//   objArray[0]->Send();
//   objArray[1]->Send();
//   objArray[2]->Send();

//   // //manual replace
//   // delete objArray[0];
//   // objArray[0] = new Chr('d');
//   // objArray[0]->Send();

//   // //global? function replace
//   // replace(objArray[0], new Chr('d'));
//   // objArray[0]->Send();
//   // replace(objArray[0], new Chr('e'));
//   // objArray[0]->Send();

//   //static class function replace
//   Command::replace(objArray[0], new Chr('d'));
//   objArray[0]->Send();
//   Command::replace(objArray[0], new Chr('e'));
//   objArray[0]->Send();

// }

// void loop()
// {

// }
