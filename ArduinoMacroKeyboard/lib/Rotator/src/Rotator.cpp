
#include "Arduino.h"

const int NUM_ROWS = 3;
const int NUM_COLS = 3;
int buttonPins[NUM_ROWS][NUM_COLS]= {
		{1, 2, 3},
		{4, 5, 6},
		{7, 8, 9}
};

int tempArray[NUM_ROWS][NUM_COLS];

int currentOrientation = 3; // 0,1,2,3 -> top, left, bottom, right
int lastOrientation = 3;
int delta = currentOrientation - lastOrientation;

typedef int (*accessfn)(int x, int y);

int normal( int x, int y){ return buttonPins[x][y]; }
int rotateCW( int x, int y){ return buttonPins[NUM_ROWS-1-y][x]; }   // <<<<<< This is the main thing
int rotateCCW( int x, int y){ return buttonPins[y][NUM_COLS-1-x]; }

void rotateMatrix( accessfn afn){
	for (int x=0; x<NUM_ROWS; x++)
  {
		for(int y=0; y<NUM_COLS; y++)
    {
      tempArray[x][y] = afn(x,y); // now lets try and only pass the pointers
    }
  }
  cloneArray();
}

void cloneArray()
{
  for (int i = 0; i < NUM_ROWS; i++)
  {
    for (int j = 0; j < NUM_COLS; j++)
    {
      buttonPins[i][j] = tempArray[i][j];
    }
  }
}

void initTempArray()
{
  byte i = NUM_ROWS, j = NUM_COLS;
  for (int i = 0; i < NUM_ROWS; i++)
  {
    for (int j = 0; j < NUM_COLS; j++)
    {
      tempArray[i][j] = buttonPins[i][j];
    }
  }
}

void checkOrientation()
{ 
  lastOrientation = currentOrientation;
  //currentOrientation = INPUT;
  delta = currentOrientation - lastOrientation;

  if(delta = 0)
    return;
  else
    if(delta > 0)
      for(int i=0; i<delta; i++)
        rotateMatrix(rotateCW);
    else
      for(int i=0; i>delta; i--)
        rotateMatrix(rotateCCW);
}

void printArray()
{
  for (int x=0; x<NUM_ROWS; x++)
  {
		for(int y=0; y<NUM_COLS; y++)
    {
      Serial.print(buttonPins[x][y]);
      delay(100);
    }
    Serial.println("");
  }
  Serial .println("");
}

void setup(){
	Serial.begin(9600);

}

void loop(){
  
}
