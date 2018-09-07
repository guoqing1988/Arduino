/*
 * 
*/

#define PIXELS_PER_BOARD  8
#define BOARD_COLUMNS     4
#define BOARD_ROWS        16
#define PIXELS_PER_ROW    (PIXELS_PER_BOARD * BOARD_COLUMNS)
#define NUM_PIXELS        (PIXELS_PER_ROW * BOARD_ROWS)

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
int clockPin        = 10; // Connects to the Clock pin the 165

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

void setup()
{
  Serial.begin(9600);

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
}

void loop()
{
  static unsigned long numLoops = 0;
  static unsigned long timerStart = millis();
  unsigned long currentTime;
  boolean needPrintln;
  
  // read the new switch values
  readSwitches();

  needPrintln = false;
  for(int i = 0; i < NUM_PIXELS; i++) {
    if(pixelPressed[i] != pixelPressedPrior[i]) {
      Serial.print(i);
      Serial.print(": ");
      Serial.print(pixelPressed[i] ? "ON " : "OFF ");
      needPrintln = true;
    }
  }
  if(needPrintln) {
    Serial.println();
  }
  
  // check to see if this is a mode switch event
  if((pixelPressed[0] == true) && (pixelPressedPrior[0] == false)) {
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

  numLoops++;
  
  delay(POLL_DELAY_MSEC);
}

