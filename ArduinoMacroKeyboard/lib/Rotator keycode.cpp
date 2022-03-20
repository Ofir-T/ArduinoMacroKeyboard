
#include "Arduino.h"
#include <HID-Project.h>

const int NUMPAD_ROWS = 3;
const int NUMPAD_COLS = 3;
int buttonPins[NUMPAD_ROWS][NUMPAD_COLS]= {
		{1, 2, 3},
		{4, 5, 6},
		{7, 8, 9}
};

KeyboardKeycode keypadSet[NUMPAD_ROWS*NUMPAD_COLS] = 
  {
    KEY_1, KEY_Q, KEY_F, KEY_D, KEY_4, KEY_W, KEY_B, KEY_E, KEY_R  // Example League of Legends set ;)
  };


KeyboardKeycode tempArray[NUMPAD_ROWS*NUMPAD_COLS];

int currentOrientation = 3; // 0,1,2,3 -> top, left, bottom, right
int lastOrientation = 3;
int delta = currentOrientation - lastOrientation;

typedef KeyboardKeycode (*accessfn)(int x, int y);

KeyboardKeycode normal( int x, int y){ return keypadSet[(NUMPAD_COLS*x)+y]; }
KeyboardKeycode rotateCW( int x, int y){ return keypadSet[(NUMPAD_COLS-1-x)+3*y]; }   // <<<<<< This is the main thing
KeyboardKeycode rotateCCW( int x, int y){ return keypadSet[3*(NUMPAD_COLS-1-y)+x]; }

void cloneArray()
{
  for (int i = 0; i < NUMPAD_ROWS*NUMPAD_COLS; i++)
  {
    keypadSet[i] = tempArray[i];
  }
}

void rotateMatrix( accessfn afn){
	for (int x=0; x<NUMPAD_ROWS; x++)
  {
		for(int y=0; y<NUMPAD_COLS; y++)
    {
      tempArray[(NUMPAD_COLS*x)+y] = afn(x,y); // now lets try and only pass the pointers
    }
  }
  cloneArray();
}

void initTempArray()
{
  for (int i = 0; i < NUMPAD_ROWS*NUMPAD_COLS; i++)
  {
      tempArray[i] = keypadSet[i];
  }
}

void printArray()
{
  for(int i=0; i<NUMPAD_COLS*NUMPAD_ROWS; i++)
  {
    Serial.print(keypadSet[i]);
    
    if((i+1) % NUMPAD_COLS == 0) 
      Serial.println("");
    else
      Serial.print(" ");

    // delay(100);
  }
  Serial.println("");
}

void checkOrientation()
{ 
  //currentOrientation = INPUT;
  delta = currentOrientation - lastOrientation;

  Serial.println("delta: " + String(delta));

  if(delta != 0)
  {
    if(delta > 0)
    {
      for(int j=0; j<delta; j++)
      {
        rotateMatrix(rotateCW);
        Serial.println("j: " + String(j));
      }
      Serial.println("rotating keypad clockwise");
      printArray();
    }
    else
    {
      for(int j=0; j>delta; j--)
      {
        rotateMatrix(rotateCCW);
        Serial.println("j: " + String(j));
      }
      Serial.println("rotating keypad counter-clockwise");
      printArray();
    }
  }

  lastOrientation = currentOrientation;
}
void setup(){
	Serial.begin(9600);
  printArray();

}

void loop(){
  
}
