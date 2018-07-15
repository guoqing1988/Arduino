/*
 * Light Bright
 * 
 * Author: Jeff Warren
*/

#include <Adafruit_GFX.h>
#include <SparseNeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <SD.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 6

#define SWITCHES_PER_BOARD 8
#define BOARDS_PER_ROW     4
#define SWITCHES_PER_ROW   (SWITCHES_PER_BOARD * BOARDS_PER_ROW)
#define BOARD_ROWS         16

#define MATRIX_WIDTH  SWITCHES_PER_ROW
#define MATRIX_HEIGHT BOARD_ROWS

#define MODE_CLEAR           0
#define MODE_PAINT           1
#define MODE_CYCLE           2
#define MODE_SCROLL_TEXT     3
#define MODE_SCREENSAVER     4
#define MODE_FADE            5
#define MODE_WIPE            6
#define MODE_EXPANDING_BOXES 7
#define MODE_COLOR_FABRIC    8
#define MODE_TIMER           9

#define MODE_CHANGE_ROW (MATRIX_HEIGHT - 1)
#define MODE_CHANGE_COL 0
#define MODE_ROW        0

unsigned char mode;

// define values for the screen saver
#define SCREEN_SAVER_TIMEOUT 10000
unsigned long screenSaverTimer = 0;

SparseNeoMatrix matrix = SparseNeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, PIN);

// defined the predefined colors (in RGB space) used for color palletes
#define NUM_COLORS 8
struct RGBColor {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} rgb_colors[] = {
  {   0,   0,   0 }, // black
  { 255,   0,   0 }, // red
  {   0, 255,   0 }, // green
  {   0,   0, 255 }, // blue
  { 192, 140,   0 }, // yellow
  { 128,   0, 128 }, // purple
  { 255, 100,   0 }, // orange
  {  84,  84,  84 }  // white
};

// declare an array that can hold the NeoPixel version of the predefined colors
uint32_t colors[NUM_COLORS];

// variable to hold the current color index (used for some modes)
unsigned char colorIndex = 0;

// pixelColors holds the current color index for each pixel
unsigned char pixelColors[MATRIX_WIDTH][MATRIX_HEIGHT];

// array that keeps track of whether the current switch just saw a rising edge
boolean pixelPressed[MATRIX_WIDTH][MATRIX_HEIGHT];
boolean pixelPressedPrior[MATRIX_WIDTH][MATRIX_HEIGHT];
boolean anyPixelPressed = false;

// only poll switches every so often as specified by numPollLoops
// this helps to debounce the switches
// pollLoop keeps track of the loop number we are currently on
unsigned int numPollLoops = 2;
unsigned int pollLoop = 0;

// Width of pulse to trigger the shift register to read and latch.
#define PULSE_WIDTH_USEC   5

// Optional delay between shift register reads
#define POLL_DELAY_MSEC   50

// delay between animation loops
#define ANIMATE_DELAY_MSEC   100

#define CHIP_SELECT_PIN 53

int ploadPin = 8;  // Connects to Parallel load pin the 165
int clockEnablePin = 9;  // Connects to Clock Enable pin the 165
int dataPin = 11; // Connects to the Q7 pin the 165
int clockPin = 10; // Connects to the Clock pin the 165

// selectedColorIndex holds the currently selected color for each of the two paint palettes
unsigned char selectedColorIndex[] {
  1, 1
};

int selectedColorBlinkIndex = 0;
#define NUM_BLINK_LOOPS 100
#define NUM_FADE_LOOPS   25

unsigned int fadeWheelIndex = 0;
unsigned char numFadeLoops = 2;
unsigned char fadeLoop = 0;

float animateRow = 0;
float animateCol = 0;
float animateSize = 0;
float animateWidth = 0;
float animateHeight = 0;
float animateDx = 0;
float animateDy = 0;

#define MAX_MESSAGE_LENGTH 127
char message[MAX_MESSAGE_LENGTH + 1];
unsigned int messageLength = 0;

#define FONT_SIZE   2
#define CHAR_WIDTH  (6 * FONT_SIZE)
#define CHAR_HEIGHT (8 * FONT_SIZE)

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

  mode = MODE_CYCLE;
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

  selectedColorBlinkIndex = 0;

  mode = MODE_PAINT;
}

void mode_paint_loop()
{
  int i, row, col;
  unsigned int red, green, blue;
  float colorPercent;

  // paint the last row of leds with both color palettes
  row = MATRIX_HEIGHT - 1;
  for(i = 0; i < 2; i++) {
    // compute the starting column location of this color palette
    col = (MATRIX_WIDTH / 2) * i;

    // draw the current color palette
    for(colorIndex = 0; colorIndex < NUM_COLORS; colorIndex++, col++) {
      // make this the selected color if this pixel was just pressed
      if((pixelPressed[col][row]) && (pixelPressedPrior[col][row] == false)) {
        selectedColorIndex[i] = colorIndex;

        selectedColorBlinkIndex = 0;
      }

      // if this is the currently selected color, compute it's color as a fading blink
      if(colorIndex == selectedColorIndex[i]) {
        colorPercent = selectedColorBlinkIndex / (float) NUM_FADE_LOOPS;
        if(colorPercent > 1) {
          colorPercent = 1;
        }
        else if(colorPercent < 0) {
          colorPercent = 0;
        }

        // if the colorIndex is 0 (black), have the color computed as a fade out from white to black,
        // otherwise, have the color computed as a fade up from black to the selected color
        if(colorIndex == 0) {
          red = 255 * (1 - colorPercent);
          green = 255 * (1 - colorPercent);
          blue = 255 * (1 - colorPercent);
        }
        else {
          red = rgb_colors[colorIndex].red * colorPercent;
          green = rgb_colors[colorIndex].green * colorPercent;
          blue = rgb_colors[colorIndex].blue * colorPercent;
        }
            
        matrix.drawPixel(col, row, matrix.Color(red, green, blue));

        selectedColorBlinkIndex++;
        if(selectedColorBlinkIndex >= NUM_BLINK_LOOPS) {
          selectedColorBlinkIndex = 0;
        }
      }
      else {
        matrix.drawPixel(col, row, colors[colorIndex]);
      }
    }
  }

  // loop through all but the last row
  for(row = 0; row < (MATRIX_HEIGHT - 1); row++) {
    for(col = 0; col < MATRIX_WIDTH; col++) {
      // check to see if this is a rising edge on this switch
      if((pixelPressedPrior[col][row] == false) && (pixelPressed[col][row] == true)) {
        // set the corresponding pixel color
        if(col < (MATRIX_WIDTH / 2)) {
          matrix.drawPixel(col, row, colors[selectedColorIndex[0]]);
        }
        else {
          matrix.drawPixel(col, row, colors[selectedColorIndex[1]]);
        }
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

void mode_color_fabric_init()
{
  clear(false);

  fadeWheelIndex = 0;

  // initiliaze the location where the central color is located
  // other pixels will compute their color relative to this location
  animateRow = MATRIX_HEIGHT / 2;
  animateCol = MATRIX_WIDTH / 2;

  animateDx = 0.5;
  animateDy = 0.3;

  // set a "size" that specifies the amount of color variation
  // as a function of distance from the central color location.
  // animateSize is in units of color wheelIndex per pixelDistance
  // The Wheel function takes values from 0 to 255 to return a color.
  // So, after computing the pixel distance, the current pixel color is computed as follows:
  // wheelIndex = (centralColorWheelIndex + (pixelDistance * animateSize)) % 255;
  animateSize = 6;
  
  mode = MODE_COLOR_FABRIC;
}

void mode_color_fabric_loop()
{
  int row, col;
  int dx, dy;
  float pixelDistance;
  uint32_t color;
  
  // loop through all the leds
  for(row = 0; row < MATRIX_HEIGHT; row++) {
    for(col = 0; col < MATRIX_WIDTH; col++) {
      // compute the distance from the central pixel location
      dx = col - animateCol;
      dy = row - animateRow;
      pixelDistance = sqrt((dx*dx) + (dy*dy));

      color = Wheel((fadeWheelIndex + (int) (pixelDistance * animateSize)) % 255);
      
      // set the corresponding pixel color
      matrix.drawPixel(col, row, color);
    }
  }
  
  // send the updated pixel colors to the NeoPixels
  matrix.show();

  animateCol += animateDx;
  if(animateCol <= 0) {
    animateCol = 0;
    animateDx = -animateDx;
  }
  else if(animateCol >= MATRIX_WIDTH) {
    animateCol = MATRIX_WIDTH - 1;
    animateDx = -animateDx;
  }

  animateRow += animateDy;
  if(animateRow <= 0) {
    animateRow = 0;
    animateDy = -animateDy;
  }
  else if(animateRow >= MATRIX_HEIGHT) {
    animateRow = MATRIX_HEIGHT - 1;
    animateDy = -animateDy;
  }
  
  // check to see if we have reached the end of the color wheel
  fadeWheelIndex += 2;
  if(fadeWheelIndex >= 255) {
    // reset back to the first color in the color wheel
    fadeWheelIndex = 0;
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
      if(colorIndex >= NUM_COLORS) {
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
    if(colorIndex >= NUM_COLORS) {
      colorIndex = 1;
    }
  }
}

void mode_scroll_text_init()
{
  matrix.setTextWrap(false);
  matrix.setTextSize(FONT_SIZE);

  animateRow = 0;
  animateCol = MATRIX_WIDTH;
  animateWidth = CHAR_WIDTH * messageLength;
  animateHeight = 1;
  colorIndex = 1;
  
  mode = MODE_SCROLL_TEXT;
}

void mode_scroll_text_loop()
{
  fillColor(colors[0]);

  matrix.setCursor(animateCol, 1);
  matrix.setTextColor(colors[colorIndex]);
  matrix.print(message);
  matrix.show();

  animateCol--;
  if(animateCol < -(animateWidth)) {
    animateCol = MATRIX_WIDTH;
    colorIndex++;
 
    // check to see if we have reached the end of all colors
    if(colorIndex >= NUM_COLORS) {
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
  int i, col, row;

  Serial.begin(9600);

  // initialize the array of colors
  for(i = 0; i < NUM_COLORS; i++) {
    colors[i] = matrix.Color(rgb_colors[i].red, rgb_colors[i].green, rgb_colors[i].blue);
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(CHIP_SELECT_PIN)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // re-open the file for reading:
  messageLength = 0;
  File myFile = SD.open("test.txt");
  if (myFile) {
    // read from the file until there's nothing else in it:
    while(myFile.available() && (messageLength < MAX_MESSAGE_LENGTH)) {
      message[messageLength++] = myFile.read();
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  message[messageLength] = '\0';

  Serial.println("message:");
  for(int i = 0; i < messageLength; i++) {
    Serial.print(message[i]);
  }
  Serial.println();

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
  pollLoop++;
  if(pollLoop >= numPollLoops) {
    readSwitches();
    pollLoop = 0;
 
    if(anyPixelPressed) {
      // reset the screen saver timer
      reset_screen_saver_timer();
  
      // check to see if this is a mode switch event
      if(pixelPressed[MODE_CHANGE_COL][MODE_CHANGE_ROW] == true) {
        if(pixelPressed[MODE_CYCLE][MODE_ROW] == true) {
          mode_cycle_init();
        }
        else if(pixelPressed[MODE_PAINT][MODE_ROW] == true) {
          mode_paint_init();
        }
        else if(pixelPressed[MODE_FADE][MODE_ROW] == true) {
          mode_fade_init();
        }
        else if(pixelPressed[MODE_EXPANDING_BOXES][MODE_ROW] == true) {
          mode_expanding_boxes_init();
        }
        else if(pixelPressed[MODE_SCROLL_TEXT][MODE_ROW] == true) {
          mode_scroll_text_init();
        }
        else if(pixelPressed[MODE_COLOR_FABRIC][MODE_ROW] == true) {
          mode_color_fabric_init();
        }
        else if(pixelPressed[MODE_CLEAR][MODE_ROW] == true) {
          clear(true);
        }
        else if((pixelPressed[MODE_TIMER][MODE_ROW] == true) && (pixelPressedPrior[MODE_TIMER][MODE_ROW] == false)) {
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
  }
  else if(millis() >= screenSaverTimer) {
    reset_screen_saver_timer();
  }

  switch(mode) {
    case MODE_CYCLE:
      mode_cycle_loop();
      break;
    case MODE_PAINT:
      mode_paint_loop();
      break;
    case MODE_FADE:
      mode_fade_loop();
      break;
    case MODE_WIPE:
      mode_wipe_loop();
      break;
    case MODE_EXPANDING_BOXES:
      mode_expanding_boxes_loop();
      break;
    case MODE_SCROLL_TEXT:
      mode_scroll_text_loop();
      break;
    case MODE_COLOR_FABRIC:
      mode_color_fabric_loop();
      break;
    default:
      break;
  }

  numLoops++;
  
  delay(delayLength);
}

