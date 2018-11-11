/*
 * Light Bright Puzzle
 * 
 * Author: Jeff Warren
*/

#include <Adafruit_GFX.h>
#include <SparseNeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include "LedControl.h"

#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 6

#define SWITCHES_PER_BOARD 8
#define BOARDS_PER_ROW     1
#define SWITCHES_PER_ROW   (SWITCHES_PER_BOARD * BOARDS_PER_ROW)
#define BOARD_ROWS         6

#define MATRIX_WIDTH  SWITCHES_PER_ROW
#define MATRIX_HEIGHT BOARD_ROWS

SparseNeoMatrix matrix = SparseNeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, PIN);

// defined the predefined colors (in RGB space) used for color palletes
#define NUM_COLORS 6
struct RGBColor {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} rgb_colors[] = {
  { 255,   0,   0 }, // 0 red
  { 255, 255,   0 }, // 1 yellow
  {   0, 255,   0 }, // 2 green
  {   0,   0, 255 }, // 3 blue
  { 153,   0, 153 }, // 4 deep violet
  {   0,   0,   0 }  // 5 black
};

// declare an array that can hold the NeoPixel version of the predefined colors
uint32_t colors[NUM_COLORS];

// variable to hold the current color index (used for some modes)
unsigned char colorIndex = 0;

// pixelColors holds the current color index for each pixel
unsigned char pixelColors[MATRIX_WIDTH][MATRIX_HEIGHT];

unsigned char pattern[MATRIX_WIDTH][MATRIX_HEIGHT];

// array that keeps track of whether the current switch just saw a rising edge
boolean pixelPressed[MATRIX_WIDTH][MATRIX_HEIGHT];
boolean pixelPressedPrior[MATRIX_WIDTH][MATRIX_HEIGHT];
boolean anyPixelPressed = false;

// only poll switches every so often as specified by numPollLoops
// this helps to debounce the switches
// pollLoop keeps track of the loop number we are currently on
unsigned int numPollLoops = 4;
unsigned int pollLoop = 0;

// Width of pulse to trigger the shift register to read and latch.
#define PULSE_WIDTH_USEC   5

// Optional delay between shift register reads
#define POLL_DELAY_MSEC   50

int ploadPin = 8;  // Connects to Parallel load pin the 165
int clockEnablePin = 9;  // Connects to Clock Enable pin the 165
int dataPin = 11; // Connects to the Q7 pin the 165
int clockPin = 10; // Connects to the Clock pin the 165

/*
 Now we need a LedControl to work with.
 pin 5 is connected to the DataIn 
 pin 3 is connected to the CLK 
 pin 4 is connected to LOAD 
 We have only a single MAX72XX.
 */
#define LED_DISPLAY_DATA_PIN  5
#define LED_DISPLAY_CLK_PIN   3
#define LED_DISPLAY_LOAD_PIN  4
LedControl lc=LedControl(LED_DISPLAY_DATA_PIN, LED_DISPLAY_CLK_PIN, LED_DISPLAY_LOAD_PIN,1);

#define NUM_DIGITS 8
String msg = "760-670-4258";

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

void clear(boolean clearBuffers)
{
  int row, col;

  if(clearBuffers) {
    // initialize the color indexes for each pixel
    for(row = 0; row < MATRIX_HEIGHT; row++) {
      for(col = 0; col < MATRIX_WIDTH; col++) {
        pixelColors[col][row] = NUM_COLORS - 1;
      }
    }
  }
  
  // clear the matrix
  matrix.clear();
  matrix.show();
}

void fillColor(uint32_t color)
{
  int row, col;

  // loop through all the leds
  for(row = 0; row < MATRIX_HEIGHT; row++) {
    for(col = 0; col < MATRIX_WIDTH; col++) {
      // set the corresponding pixel color
      matrix.drawPixel(col, row, color);
    }
  }
}

void readSwitches()
{
  int row, col, boardIndex, switchIndex;
  int pixelIndex;

  // Trigger a parallel Load to latch the state of the shift register data lines
  digitalWrite(clockEnablePin, HIGH);
  digitalWrite(ploadPin, LOW);
  delayMicroseconds(PULSE_WIDTH_USEC);
  digitalWrite(ploadPin, HIGH);
  digitalWrite(clockEnablePin, LOW);

  anyPixelPressed = false;
  for(row = 0; row < BOARD_ROWS; row++) {
    for(boardIndex = 0; boardIndex < BOARDS_PER_ROW; boardIndex++) {
      for(switchIndex  = 0; switchIndex < SWITCHES_PER_BOARD; switchIndex++) {
        col = (boardIndex * SWITCHES_PER_BOARD);
        col += switchIndex;

        // the matrix coordinates are left to right for all rows, but the switches are arranged
        // right to left for odd numbered rows. So invert the matrix column index for odd
        // numbered rows
        if((row % 2) != 0) {
           col = (MATRIX_WIDTH - 1) - col;
        }
        
        // save the prior pressed state for this pixel
        pixelPressedPrior[col][row] = pixelPressed[col][row];
        
        // read the next bit from the shift register and update the pixelPressed array
        pixelPressed[col][row] = ((digitalRead(dataPin)) == HIGH ? true : false);

        // check to see if this is a rising edge press
        if((pixelPressedPrior[col][row] == false) && (pixelPressed[col][row] == true)) {
          anyPixelPressed = true;
        }
    
        // Pulse the Clock (rising edge shifts the registers to read the next bit
        digitalWrite(clockPin, HIGH);
        delayMicroseconds(PULSE_WIDTH_USEC);
        digitalWrite(clockPin, LOW);  
      } /* end of board */
    } /* end of columns */
  } /* end of rows */
}

void mode_cycle_init()
{
  clear(false);
}

void mode_cycle_loop()
{
  int row, col;

  for(row = 0; row < MATRIX_HEIGHT; row++) {
    for(col = 0; col < MATRIX_WIDTH; col++) {
      // check to see if this is a rising edge on this switch
      if((pixelPressedPrior[col][row] == false) && (pixelPressed[col][row] == true)) {
        // increment the color for this pixel
        pixelColors[col][row] += 1;
        if(pixelColors[col][row] >= NUM_COLORS) {
            pixelColors[col][row] = 0;
        }

        // clear the rising edge on this switch so that it won't get processed again
        pixelPressedPrior[col][row] = pixelPressed[col][row];
      }

      // set the corresponding pixel color
      // Note: skip every other pixel, starting with the 2nd pixel (index = 1)
      matrix.drawPixel(col, row, colors[pixelColors[col][row]]);
    }
  }
          
  // send the updated pixel colors to the NeoPixels
  matrix.show();
}

void drawPattern()
{
  matrix.clear();
  
  for(int row = 0; row < MATRIX_HEIGHT; row++) {
    for(int col = 0; col < MATRIX_WIDTH; col++) {
      // set the corresponding pixel color
      // Note: skip every other pixel, starting with the 2nd pixel (index = 1)
      matrix.drawPixel(col, row, colors[pattern[col][row]]);
    }    
  }
  matrix.show();
}

void initPattern()
{
  // row 1
  pattern[0][0] = 2;
  pattern[1][0] = 2;
  pattern[2][0] = 1;
  pattern[3][0] = 1;
  pattern[4][0] = 0;
  pattern[5][0] = 0;
  pattern[6][0] = 0;
  pattern[7][0] = 0;

  // row 2
  pattern[0][1] = 4;
  pattern[1][1] = 2;
  pattern[2][1] = 2;
  pattern[3][1] = 1;
  pattern[4][1] = 1;
  pattern[5][1] = 0;
  pattern[6][1] = 0;
  pattern[7][1] = 0;

  // row 3
  pattern[0][2] = 4;
  pattern[1][2] = 4;
  pattern[2][2] = 2;
  pattern[3][2] = 2;
  pattern[4][2] = 1;
  pattern[5][2] = 1;
  pattern[6][2] = 0;
  pattern[7][2] = 0;

  // row 4
  pattern[0][3] = 3;
  pattern[1][3] = 4;
  pattern[2][3] = 4;
  pattern[3][3] = 2;
  pattern[4][3] = 2;
  pattern[5][3] = 1;
  pattern[6][3] = 1;
  pattern[7][3] = 0;

  // row 5
  pattern[0][4] = 3;
  pattern[1][4] = 3;
  pattern[2][4] = 4;
  pattern[3][4] = 4;
  pattern[4][4] = 2;
  pattern[5][4] = 2;
  pattern[6][4] = 1;
  pattern[7][4] = 1;

  // row 6
  pattern[0][5] = 3;
  pattern[1][5] = 3;
  pattern[2][5] = 3;
  pattern[3][5] = 4;
  pattern[4][5] = 4;
  pattern[5][5] = 2;
  pattern[6][5] = 2;
  pattern[7][5] = 1;
}
void setup()
{
  int i, col, row;

  Serial.begin(9600);

  // initialize the array of colors
  for(i = 0; i < NUM_COLORS; i++) {
    colors[i] = matrix.Color(rgb_colors[i].red, rgb_colors[i].green, rgb_colors[i].blue);
  }

  // initialize the neoPixel matrix
  matrix.begin();
  
  clear(true);
  
  // Initialize the shift registers
  pinMode(ploadPin, OUTPUT);
  pinMode(clockEnablePin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);
  
  digitalWrite(clockPin, LOW);
  digitalWrite(ploadPin, HIGH);
  
  // initialize the switch states
  for(row = 0; row < MATRIX_HEIGHT; row++) {
    for(col = 0; col < MATRIX_WIDTH; col++) {
      pixelPressed[col][row] = false;
      pixelPressedPrior[col][row] = false;
    }
  }

  // set the default mode to cycle
  mode_cycle_init();

  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
  */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0, 8);

  /* and clear the display */
  lc.clearDisplay(0);

  initPattern();
  
//  drawPattern();
}

bool checkPattern()
{
  bool found = true;
  
  for(int row = 0; row < MATRIX_HEIGHT; row++) {
    for(int col = 0; col < MATRIX_WIDTH; col++) {
      // check to see if the secret code was found
      if(pixelColors[col][row] != pattern[col][row]) {
        found = false;
      }
    }    
  }

  return found;
}

void loop()
{
  static bool found = false;
  
  if(found == false) {
    // read the new switch values
    pollLoop++;
    if(pollLoop >= numPollLoops) {
      readSwitches();
      pollLoop = 0; 
    }

    mode_cycle_loop();

    // check to see if the secret pattern was found
    found = checkPattern();
  }

  if(found) {
    scrollMessage();
  }
}
