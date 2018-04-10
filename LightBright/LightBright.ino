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

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

#define PIXELS_PER_BOARD  8
#define BOARD_COLUMNS     4
#define BOARD_ROWS        16  
#define PIXELS_PER_ROW    (PIXELS_PER_BOARD * BOARD_COLUMNS)
#define NUM_PIXELS        (PIXELS_PER_ROW * BOARD_ROWS)
#define LEDS_PER_ROW      ((PIXELS_PER_ROW * 2) - 1)
#define NUM_LEDS          (LEDS_PER_ROW * BOARD_ROWS)

#define MODE_COLOR_CYCLE  0
#define MODE_PAINT        1
#define MODE_FADE         2
#define MODE_WIPE         3

#define MODE_CHANGE_INDEX       (NUM_PIXELS - 1)
#define MODE_CLEAR_INDEX        0
#define MODE_PAINT_INDEX        1
#define MODE_COLOR_CYCLE_INDEX  2
#define MODE_FADE_INDEX         3
#define MODE_TIMER_INDEX        4

unsigned char mode;

#define BLINK_WHITE_MILLIS  200
#define BLINK_COLOR_MILLIS  1500
boolean blinkWhite = false;

// remember the millis() time when we should switch to the next blink state
unsigned long blinkMillis = 0;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel leds = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// colors is an array of colors that can be cycled for each pixel
uint32_t colors[] = {
  leds.Color(0,0,0),      // black
  leds.Color(255,0,0),    // red
  leds.Color(0,255,0),    // green
  leds.Color(0,0,255),    // blue
  leds.Color(192,140,0),  // yellow
  leds.Color(128,0,128),  // purple
  leds.Color(255,100,0),  // orange
  leds.Color(84,84,84) // white
};

// determine the number of colors in colors array
unsigned char numColors = sizeof(colors) / sizeof(uint32_t);

// variable to hold the current color index (used for some modes)
unsigned char currentColorIndex = 0;

// pixelColors holds the current color index for each pixel
unsigned char pixelColors[NUM_PIXELS];

// array that keeps track of whether the current switch just saw a rising edge
boolean pixelPressed[NUM_PIXELS];
boolean pixelPressedPrior[NUM_PIXELS];

/* Width of pulse to trigger the shift register to read and latch.
*/
#define PULSE_WIDTH_USEC   5

/* Optional delay between shift register reads.
*/
#define POLL_DELAY_MSEC   50

int ploadPin        = 8;  // Connects to Parallel load pin the 165
int clockEnablePin  = 9;  // Connects to Clock Enable pin the 165
int dataPin         = 11; // Connects to the Q7 pin the 165
int clockPin        = 12; // Connects to the Clock pin the 165

unsigned char fadeWheelIndex = 0;
unsigned char numFadeLoops = 2;
unsigned char fadeLoop = 0;

unsigned int wipeRowIndex = 0;
unsigned int wipeRowLEDIndex = 0;
unsigned int wipeLEDIndex = 0;
unsigned int wipeColorIndex = 1;

void clear()
{
  /*
   * initialize the color indexes for each pixel and set the corresponding
   * pixel to that color
  */
  for(int i = 0; i < NUM_PIXELS; i++) {
    pixelColors[i] = 0;
    leds.setPixelColor((i * 2), colors[pixelColors[i]]);
    leds.setPixelColor((i * 2) + 1, colors[pixelColors[i]]);
  }
  
  // display the pixels
  leds.show();
}

void readSwitches()
{
  int rowIndex, colIndex, boardIndex;
  int pixelIndex;

  // Trigger a parallel Load to latch the state of the shift register data lines
  digitalWrite(clockEnablePin, HIGH);
  digitalWrite(ploadPin, LOW);
  delayMicroseconds(PULSE_WIDTH_USEC);
  digitalWrite(ploadPin, HIGH);
  digitalWrite(clockEnablePin, LOW);

  pixelIndex = 0;
  for(rowIndex = 0; rowIndex < BOARD_ROWS; rowIndex++) {
    for(colIndex = 0; colIndex < BOARD_COLUMNS; colIndex++) {
      for(boardIndex = 0; boardIndex < PIXELS_PER_BOARD; boardIndex++, pixelIndex++) {
        // save the prior pressed state for this pixel
        pixelPressedPrior[pixelIndex] = pixelPressed[pixelIndex];
        
        // read the next bit from the shift register and update the pixelPressed array
        pixelPressed[pixelIndex] = ((digitalRead(dataPin)) == HIGH ? true : false);
    
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
  clear();

  mode = MODE_COLOR_CYCLE;
}

void mode_color_cycle_loop()
{
  int rowIndex, colIndex, boardIndex;
  int ledIndex, pixelIndex;

  pixelIndex = 0;
  ledIndex = 0;
  for(rowIndex = 0; rowIndex < BOARD_ROWS; rowIndex++) {
    for(colIndex = 0; colIndex < BOARD_COLUMNS; colIndex++) {
      for(boardIndex = 0; boardIndex < PIXELS_PER_BOARD; boardIndex++, pixelIndex++, ledIndex += 2) {
        // check to see if this is a rising edge on this switch
        if((pixelPressedPrior[pixelIndex] == false) && (pixelPressed[pixelIndex] == true)) {
          // increment the color for this pixel
          pixelColors[pixelIndex] += 1;
          if(pixelColors[pixelIndex] >= numColors) {
              pixelColors[pixelIndex] = 0;
          }

          // set the corresponding pixel color
          // Note: skip every other pixel, starting with the 2nd pixel (index = 1)
          leds.setPixelColor(ledIndex, colors[pixelColors[pixelIndex]]);
        }
      } /* end of board */
    } /* end of columns */

    ledIndex--;
  } /* end of rows */
          
  // send the updated pixel colors to the NeoPixels
  leds.show();
}

void mode_paint_init()
{
  clear();

  currentColorIndex = 1;

  blinkWhite = true;
  blinkMillis = millis() + BLINK_WHITE_MILLIS;

  mode = MODE_PAINT;
}

void mode_paint_loop()
{
  int rowIndex, colIndex, boardIndex;
  int ledIndex, pixelIndex;

  // paint the last row of leds with the color pallette
  rowIndex = BOARD_ROWS - 1;
  pixelIndex = (PIXELS_PER_ROW * rowIndex);
  ledIndex = (LEDS_PER_ROW * rowIndex);

  for(colIndex = 0; ((colIndex < numColors) && (colIndex < PIXELS_PER_ROW)); colIndex++, pixelIndex++, ledIndex += 2) {
    // check to see if the blink alarm needs to be updated
    if(millis() >= blinkMillis) {
      if(blinkWhite) {
        blinkWhite = false;
        blinkMillis = millis() + BLINK_COLOR_MILLIS;
      }
      else {
        blinkWhite = true;
        blinkMillis = millis() + BLINK_WHITE_MILLIS;
      }
    }
    
    if((blinkWhite) && (currentColorIndex == colIndex)) {
      leds.setPixelColor(ledIndex, leds.Color(192,192,192));
    }
    else {
      leds.setPixelColor(ledIndex, colors[colIndex]);
    }

    // check to see if this switch is pressed
    if(pixelPressed[pixelIndex] == true) {
      currentColorIndex = colIndex;

      blinkWhite = true;
      blinkMillis = millis() + BLINK_WHITE_MILLIS;
    }
  }

  pixelIndex = 0;
  ledIndex = 0;
  // loop through all but the last row
  for(rowIndex = 0; rowIndex < (BOARD_ROWS - 1); rowIndex++) {
    for(colIndex = 0; colIndex < BOARD_COLUMNS; colIndex++) {
      for(boardIndex = 0; boardIndex < PIXELS_PER_BOARD; boardIndex++, pixelIndex++, ledIndex += 2) {
        // check to see if this is a rising edge on this switch
        if((pixelPressedPrior[pixelIndex] == false) && (pixelPressed[pixelIndex] == true)) {
          // set the corresponding pixel color
          leds.setPixelColor(ledIndex, colors[currentColorIndex]);
        }
      } /* end of board */
    } /* end of columns */

    ledIndex--;
  } /* end of rows */
          
  // send the updated pixel colors to the NeoPixels
  leds.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return leds.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return leds.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return leds.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void mode_fade_init()
{
  clear();

  fadeWheelIndex = 0;
  fadeLoop = 0;
  
  mode = MODE_FADE;
}

void mode_fade_loop()
{
  int rowIndex, colIndex, boardIndex;
  int ledIndex = 0;

  uint32_t color = Wheel(fadeWheelIndex);
  
  // loop through all the leds
  for(rowIndex = 0; rowIndex < BOARD_ROWS; rowIndex++) {
    for(colIndex = 0; colIndex < BOARD_COLUMNS; colIndex++) {
      for(boardIndex = 0; boardIndex < PIXELS_PER_BOARD; boardIndex++, ledIndex += 2) {
        // set the corresponding pixel color
        leds.setPixelColor(ledIndex, color);
      } /* end of board */
    } /* end of columns */

    ledIndex--;
  } /* end of rows */
          
  // send the updated pixel colors to the NeoPixels
  leds.show();

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
  clear();

  wipeRowIndex = 0;
  wipeRowLEDIndex = 0;
  wipeLEDIndex = 0;
  wipeColorIndex = 1;
  
  mode = MODE_WIPE;
}

void mode_wipe_loop()
{
  leds.setPixelColor(wipeLEDIndex, colors[wipeColorIndex]);
  leds.show();

  // increment the LED index for the current row
  wipeRowLEDIndex += 2;

  // check to see if we have reached the end of the row
  if(wipeRowLEDIndex >= LEDS_PER_ROW) {
    wipeRowIndex++;
    wipeRowLEDIndex = 0;
    wipeLEDIndex += 1;

    // check to see if we have reached the end of all rows
    if(wipeRowIndex >= BOARD_ROWS) {
      // increment the color index
      wipeColorIndex++;

      wipeRowIndex = 0;
      wipeLEDIndex = 0;

      // check to see if we have reached the end of all colors
      if(wipeColorIndex >= numColors) {
        wipeColorIndex = 1;

        // switch to fade mode
        mode_fade_init();
      }
    }
  }
  else {
    wipeLEDIndex += 2;
  }
}

void setup()
{
  Serial.begin(9600);

  // initialize the neoPixel library
  leds.begin();
  
  clear();
  
  // Initialize the shift registers
  pinMode(ploadPin, OUTPUT);
  pinMode(clockEnablePin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);
  
  digitalWrite(clockPin, LOW);
  digitalWrite(ploadPin, HIGH);
  
  // initialize the old switch states
  for(int i = 0; i < NUM_PIXELS; i++) {
    pixelPressed[i] = false;
    pixelPressedPrior[i] = false;
  }

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

  // check to see if this is a mode switch event
  if(pixelPressed[MODE_CHANGE_INDEX] == true) {
    if(pixelPressed[MODE_COLOR_CYCLE_INDEX] == true) {
      mode_color_cycle_init();
    }
    else if(pixelPressed[MODE_PAINT_INDEX] == true) {
      mode_paint_init();
    }
    else if(pixelPressed[MODE_FADE_INDEX] == true) {
      mode_fade_init();
    }
    else if(pixelPressed[MODE_CLEAR_INDEX] == true) {
      clear();
    }
    else if((pixelPressed[MODE_TIMER_INDEX] == true) && (pixelPressedPrior[MODE_TIMER_INDEX] == false)) {
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
    default:
      break;
  }

  numLoops++;
  
  delay(POLL_DELAY_MSEC);
}

