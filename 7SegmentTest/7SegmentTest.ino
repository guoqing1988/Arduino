//We always have to include the library
#include "LedControl.h"

/*
 Now we need a LedControl to work with.
 pin 12 is connected to the DataIn 
 pin 10 is connected to the CLK 
 pin 11 is connected to LOAD 
 We have only a single MAX72XX.
 */
#define NUM_BOARDS 1
LedControl lc=LedControl(7,5,6,NUM_BOARDS); // DIN, CLK, LOAD (or CS)

#define NUM_DIGITS 16
String msg = "760-518-1356";

/* we always wait a bit between updates of the display */
unsigned long delaytime=2000;

void displayChar(int index, byte val) {
  int board = index / 8;
  int boardIndex = (7 - (index % 8));
  Serial.print(board);
  Serial.print(':');
  Serial.print(boardIndex);
  Serial.print(' ');
  if((index < 0) || (index >= NUM_DIGITS)) {
    return;
  }
  lc.setChar(board, boardIndex, val, false);
}

void scrollMessage() {
  Serial.print("scroll message: length = ");
  Serial.println(msg.length());
  int len = msg.length();
  for(int startIndex = -NUM_DIGITS; startIndex < len; startIndex++) {
    Serial.print("startIndex = ");
    Serial.println(startIndex);
    for(int i = 0, index = startIndex; i < NUM_DIGITS; i++, index++) {
      if((index < 0) || (index >= msg.length())) {
        displayChar(i, ' ');
      }
      else {
        displayChar(i, msg.charAt(index));
      }
    }
    Serial.println();
    delay(500);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Hello World");

  for(int i = 0; i < NUM_BOARDS; i++) {
    /*
     The MAX72XX is in power-saving mode on startup,
     we have to do a wakeup call
     */
    lc.shutdown(i,false);
    /* Set the brightness to a medium values */
    lc.setIntensity(i,8);
    /* and clear the display */
    lc.clearDisplay(i);
  }
}


/*
 This method will display the characters for the
 word "Arduino" one after the other on digit 0. 
 */
void writeArduinoOn7Segment() {
  lc.setChar(0,0,'a',false);
  delay(delaytime);
  lc.setRow(0,0,0x05);
  delay(delaytime);
  lc.setChar(0,0,'d',false);
  delay(delaytime);
  lc.setRow(0,0,0x1c);
  delay(delaytime);
  lc.setRow(0,0,B00010000);
  delay(delaytime);
  lc.setRow(0,0,0x15);
  delay(delaytime);
  lc.setRow(0,0,0x1D);
  delay(delaytime);
  lc.clearDisplay(0);
  delay(delaytime);
} 

/*
  This method will scroll all the hexa-decimal
 numbers and letters on the display. You will need at least
 four 7-Segment digits. otherwise it won't really look that good.
 */
void scrollDigits() {
  for(int i=0;i<9;i++) {
    lc.setDigit(0,7,i,false);
    lc.setDigit(0,6,i+1,false);
    lc.setDigit(0,5,i+2,false);
    lc.setDigit(0,4,i+3,false);
    lc.setDigit(0,3,i+4,false);
    lc.setDigit(0,2,i+5,false);
    lc.setDigit(0,1,i+6,false);
    lc.setDigit(0,0,i+7,false);
    delay(delaytime);
  }
  lc.clearDisplay(0);
  delay(delaytime);
}

void testDisplay() {
  for(int board = 0; board < NUM_BOARDS; board++) {
    for(int i = 0; i < 8; i++) {
      lc.setDigit(board, i, board+i, false);
    }
  }
  delay(delaytime);
  for(int board = 0; board < NUM_BOARDS; board++) {
    lc.clearDisplay(board);
  }
  delay(delaytime);
}

void loop() { 
  writeArduinoOn7Segment();
//  scrollDigits();
//  testDisplay();
//  scrollMessage();
}
