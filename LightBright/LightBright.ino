/*
 * SN74HC165N_shift_reg
 *
 * Program to shift in the bit values from a SN74HC165N 8-bit
 * parallel-in/serial-out shift register.
 *
 * This sketch demonstrates reading in 16 digital states from a
 * pair of daisy-chained SN74HC165N shift registers while using
 * only 4 digital pins on the Arduino.
 *
 * You can daisy-chain these chips by connecting the serial-out
 * (Q7 pin) on one shift register to the serial-in (Ds pin) of
 * the other.
 * 
 * Of course you can daisy chain as many as you like while still
 * using only 4 Arduino pins (though you would have to process
 * them 4 at a time into separate unsigned long variables).
 * 
*/

// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_GFX.h>
#include <SparseNeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

#define SWITCHES_PER_BOARD 8
#define BOARDS_PER_ROW     4
#define SWITCHES_PER_ROW   (SWITCHES_PER_BOARD * BOARDS_PER_ROW)
#define BOARD_ROWS         16

#define MATRIX_WIDTH  SWITCHES_PER_ROW
#define MATRIX_HEIGHT BOARD_ROWS

#define MODE_COLOR_CYCLE     0
#define MODE_PAINT           1
#define MODE_FADE            2
#define MODE_WIPE            3
#define MODE_EXPANDING_BOXES 4

#define MODE_CHANGE_ROW (MATRIX_HEIGHT - 1)
#define MODE_CHANGE_COL 0
#define MODE_ROW        0
#define MODE_CLEAR_COL  0
#define MODE_PAINT_COL  1
#define MODE_CYCLE_COL  2
#define MODE_FADE_COL   3
#define MODE_EXPANDING_BOXES_COL 4
#define MODE_TIMER_COL  5

unsigned char mode;

// define values for blinking leds
#define BLINK_WHITE_TIMEOUT  200
#define BLINK_COLOR_TIMEOUT  1500
boolean blinkWhite = false;
unsigned long blinkTimer = 0;

// define values for the screen saver
#define SCREEN_SAVER_TIMEOUT 10000
unsigned long screenSaverTimer = 0;

SparseNeoMatrix matrix = SparseNeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, PIN);

// colors is an array of colors that can be cycled for each pixel
uint32_t colors[] = {
  matrix.Color(0,0,0),      // black
  matrix.Color(255,0,0),    // red
  matrix.Color(0,255,0),    // green
  matrix.Color(0,0,255),    // blue
  matrix.Color(192,140,0),  // yellow
  matrix.Color(128,0,128),  // purple
  matrix.Color(255,100,0),  // orange
  matrix.Color(84,84,84) // white
};

// determine the number of colors in colors array
unsigned char numColors = sizeof(colors) / sizeof(uint32_t);

// variable to hold the current color index (used for some modes)
unsigned char colorIndex = 0;

// pixelColors holds the current color index for each pixel
unsigned char pixelColors[MATRIX_WIDTH][MATRIX_HEIGHT];

// array that keeps track of whether the current switch just saw a rising edge
boolean pixelPressed[MATRIX_WIDTH][MATRIX_HEIGHT];
boolean pixelPressedPrior[MATRIX_WIDTH][MATRIX_HEIGHT];
boolean anyPixelPressed = false;

// Width of pulse to trigger the shift register to read and latch.
#define PULSE_WIDTH_USEC   5

// Optional delay between shift register reads
#define POLL_DELAY_MSEC   50

// delay between animation loops
#define ANIMATE_DELAY_MSEC   100

int ploadPin = 8;  // Connects to Parallel load pin the 165
int clockEnablePin = 9;  // Connects to Clock Enable pin the 165
int dataPin = 11; // Connects to the Q7 pin the 165
int clockPin = 12; // Connects to the Clock pin the 165

unsigned char fadeWheelIndex = 0;
unsigned char numFadeLoops = 2;
unsigned char fadeLoop = 0;

int animateRow = 0;
int animateCol = 0;
int animateSize = 0;
int animateWidth = 0;
int animateHeight = 0;

void clear(boolean clearBuffers)
{
  int row, col;

  if(clearBuffers) {
    // initialize the color indexes for each pixel
    for(row = 0; row < MATRIX_HEIGHT; row++) {
      for(col = 0; col < MATRIX_WIDTH; col++) {
        pixelColors[col][row] = 0;
      }
    }
  }
  
  // clear the matrix
  matrix.clear();
  matrix.show();
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

void mode_color_cycle_init()
{
  clear(false);

  mode = MODE_COLOR_CYCLE;
}

void mode_color_cycle_loop()
{
  int row, col;

  for(row = 0; row < MATRIX_HEIGHT; row++) {
    for(col = 0; col < MATRIX_WIDTH; col++) {
      // check to see if this is a rising edge on this switch
      if((pixelPressedPrior[col][row] == false) && (pixelPressed[col][row] == true)) {
        // increment the color for this pixel
        pixelColors[col][row] += 1;
        if(pixelColors[col][row] >= numColors) {
            pixelColors[col][row] = 0;
        }
      }

      // set the corresponding pixel color
      // Note: skip every other pixel, starting with the 2nd pixel (index = 1)
      matrix.drawPixel(col, row, colors[pixelColors[col][row]]);
    }
  }
          
  // send the updated pixel colors to the NeoPixels
  matrix.show();
}

void mode_paint_init()
{
  clear(false);

  colorIndex = 1;

  blinkWhite = true;
  blinkTimer = millis() + BLINK_WHITE_TIMEOUT;

  mode = MODE_PAINT;
}

void mode_paint_loop()
{
  int row, col;

  // check to see if the blink alarm needs to be updated
  if(millis() >= blinkTimer) {
    if(blinkWhite) {
      blinkWhite = false;
      blinkTimer = millis() + BLINK_COLOR_TIMEOUT;
    }
    else {
      blinkWhite = true;
      blinkTimer = millis() + BLINK_WHITE_TIMEOUT;
    }
  }

  // paint the last row of leds with the color pallette
  row = MATRIX_HEIGHT - 1;
  for(col = 0; ((col < numColors) && (col < MATRIX_WIDTH)); col++) { 
    if((blinkWhite) && (colorIndex == col)) {
      matrix.drawPixel(col, row, matrix.Color(192,192,192));
    }
    else {
      matrix.drawPixel(col, row, colors[col]);
    }

    // check to see if this switch is pressed
    if(pixelPressed[col][row] == true) {
      colorIndex = col;

      blinkWhite = true;
      blinkTimer = millis() + BLINK_WHITE_TIMEOUT;
    }
  }

  // loop through all but the last row
  for(row = 0; row < (MATRIX_HEIGHT - 1); row++) {
    for(col = 0; col < MATRIX_WIDTH; col++) {
      // check to see if this is a rising edge on this switch
      if((pixelPressedPrior[col][row] == false) && (pixelPressed[col][row] == true)) {
        // set the corresponding pixel color
        matrix.drawPixel(col, row, colors[colorIndex]);
      }
    }
  }
          
  // send the updated pixel colors to the NeoPixels
  matrix.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return matrix.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return matrix.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return matrix.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void mode_fade_init()
{
  clear(false);

  fadeWheelIndex = 0;
  fadeLoop = 0;
  
  mode = MODE_FADE;
}

void mode_fade_loop()
{
  int row, col;

  uint32_t color = Wheel(fadeWheelIndex);
  
  // loop through all the leds
  for(row = 0; row < MATRIX_HEIGHT; row++) {
    for(col = 0; col < MATRIX_WIDTH; col++) {
      // set the corresponding pixel color
      matrix.drawPixel(col, row, color);
    }
  }
          
  // send the updated pixel colors to the NeoPixels
  matrix.show();

  // check to see if we have reached the end of the color wheel
  if(fadeWheelIndex == 255) {
    // reset back to the first color in the color wheel
    fadeWheelIndex = 0;

    // increment the fade loop count
    fadeLoop++;

    // if we have reached the maximum number of fade loops, switch to wipe mode
    if(fadeLoop >= numFadeLoops) {
      mode_wipe_init();
    }
  }
  else {
    fadeWheelIndex++;
  }
}

void mode_wipe_init()
{
  clear(false);

  animateRow = 0;
  animateCol = 0;
  colorIndex = 1;
  
  mode = MODE_WIPE;
}

void mode_wipe_loop()
{
  matrix.drawPixel(animateCol, animateRow, colors[colorIndex]);
  matrix.show();

  // increment the column index for the current row
  animateCol++;

  // check to see if we have reached the end of the row
  if(animateCol >= MATRIX_WIDTH) {
    animateRow++;
    animateCol = 0;

    // check to see if we have reached the end of all rows
    if(animateRow >= MATRIX_HEIGHT) {
      // increment the color index
      colorIndex++;

      animateRow = 0;

      // check to see if we have reached the end of all colors
      if(colorIndex >= numColors) {
        colorIndex = 1;

        // switch to fade mode
        mode_fade_init();
      }
    }
  }
}

void mode_expanding_boxes_init()
{
  animateRow = (MATRIX_HEIGHT / 2) - 1;
  animateCol = (MATRIX_WIDTH / 2) - 1;
  animateWidth = 2;
  animateHeight = 2;
  colorIndex = 1;
  
  mode = MODE_EXPANDING_BOXES;
}

void mode_expanding_boxes_loop()
{
  matrix.clear();
  matrix.drawRect(animateCol, animateRow, animateWidth, animateHeight, colors[colorIndex]);
  matrix.show();

  // increment the column index for the current row
  animateCol -= 2;
  animateRow--;
  animateWidth += 4;
  animateHeight += 2;

  // check to see if we have reached the end of the row
  if(animateRow < 0) {
    animateRow = (MATRIX_HEIGHT / 2) - 1;
    animateCol = (MATRIX_WIDTH / 2) - 1;
    animateWidth = 2;
    animateHeight = 2;

    // increment the color index
    colorIndex++;

    // check to see if we have reached the end of all colors
    if(colorIndex >= numColors) {
      colorIndex = 1;
    }
  }
}

void show_white()
{
  int row, col;

  uint32_t color = matrix.Color(255, 255, 255);
  
  // loop through all the leds
  for(row = 0; row < MATRIX_HEIGHT; row++) {
    for(col = 0; col < MATRIX_WIDTH; col++) {
      // set the corresponding pixel color
      matrix.drawPixel(col, row, color);
    }
  }
          
  // send the updated pixel colors to the NeoPixels
  matrix.show();
}

void reset_screen_saver_timer()
{
  // initialize the screen saver timeout
  screenSaverTimer = millis() + SCREEN_SAVER_TIMEOUT;
}

void setup()
{
  int col, row;

  Serial.begin(9600);

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

  reset_screen_saver_timer();

  // set the default mode to paint
  mode_paint_init();
}

void loop()
{
  static unsigned long numLoops = 0;
  static unsigned long timerStart = millis();
  unsigned long currentTime;
  unsigned int delayLength;
  
  // read the new switch values
  readSwitches();
 
  if(anyPixelPressed) {
    // reset the screen saver timer
    reset_screen_saver_timer();

    // check to see if this is a mode switch event
    if(pixelPressed[MODE_CHANGE_COL][MODE_CHANGE_ROW] == true) {
      if(pixelPressed[MODE_CYCLE_COL][MODE_ROW] == true) {
        mode_color_cycle_init();
      }
      else if(pixelPressed[MODE_PAINT_COL][MODE_ROW] == true) {
        mode_paint_init();
      }
      else if(pixelPressed[MODE_FADE_COL][MODE_ROW] == true) {
        mode_fade_init();
      }
      else if(pixelPressed[MODE_EXPANDING_BOXES_COL][MODE_ROW] == true) {
        mode_expanding_boxes_init();
      }
      else if(pixelPressed[MODE_CLEAR_COL][MODE_ROW] == true) {
        clear(true);
      }
      else if((pixelPressed[MODE_TIMER_COL][MODE_ROW] == true) && (pixelPressedPrior[MODE_TIMER_COL][MODE_ROW] == false)) {
        currentTime = millis();
        
        Serial.print("# loops = ");
        Serial.print(numLoops);
        Serial.print(" duration = ");
        Serial.print(currentTime - timerStart);
        Serial.print(" avg loop time = ");
        Serial.println((currentTime - timerStart) / (double) numLoops);
  
        numLoops = 0;
        timerStart = currentTime;
      }
    }
    else {
      // if the screen saver was active, and this is a pixel press, then switch to paint mode
      if(mode == MODE_EXPANDING_BOXES) {
        mode_paint_init();
      }
    }
  }
  else if(millis() >= screenSaverTimer) {
    reset_screen_saver_timer();

    if(mode != MODE_EXPANDING_BOXES) {
      mode_expanding_boxes_init();
    }
  }

  delayLength = 0;
  switch(mode) {
    case MODE_COLOR_CYCLE:
      mode_color_cycle_loop();
      delayLength = POLL_DELAY_MSEC;
      break;
    case MODE_PAINT:
      mode_paint_loop();
      delayLength = POLL_DELAY_MSEC;
      break;
    case MODE_FADE:
      mode_fade_loop();
      break;
    case MODE_WIPE:
      mode_wipe_loop();
      break;
    case MODE_EXPANDING_BOXES:
      mode_expanding_boxes_loop();
      // show_white();
      delayLength = ANIMATE_DELAY_MSEC;
      break;
    default:
      break;
  }

  numLoops++;
  
  delay(delayLength);
}

