#include <SoftwareSerial.h>
#include "LedControl.h"

// define the 7 segment LED display
#define NUM_LED_BOARDS 1
int LEDDataInPin = 7;
int LEDClkPin = 5;
int LEDLoadPin = 6;
LedControl lc=LedControl(LEDDataInPin,LEDClkPin,LEDLoadPin,NUM_LED_BOARDS);

// SoftwareSerial takes two parameters in the following order: RX pin, TX pin
// These are the pins that the Arduino will use to receive and transmit (repsectively)
// The Arduino RX pin should be connected to the Bluetooth TX pin
// The Arduino TX pin should be connected to the Bluetooth RX pin
// Therefore, it's easiest define variables to define which Arduino pins
// the bluetooth RX and TX pins are connected to.
// Then the SoftwareSerial port will be defined using the bluetoothTX pin
// for the Arduino RX pin, and the bluetoothRX pin for the Arduino TX pin.
int bluetoothTX = 10; // Note: pin 10 for the Arduino RX pin works on Uno, Mega, and Leonardo
int bluetoothRX = 11;
SoftwareSerial BT(bluetoothTX, bluetoothRX);

#define NUM_BUTTONS (2)
int buttonPins[NUM_BUTTONS] = { 3, 2 };

bool buttonState[NUM_BUTTONS];
bool lastButtonState[NUM_BUTTONS];
unsigned long lastDebounceTime[NUM_BUTTONS]; // the last time the output pin was toggled

unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

int count = 0;

void displayCount() {
  // clear the display
  lc.clearDisplay(0);

  lc.setDigit(0, 0, count % 10, false);
  if(count >= 10) {
    lc.setDigit(0, 1, count / 10, false);
  }
}

void setup()
{
  // set up the pins for the buttons and initialize their prior state
  for(int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);

    buttonState[i] = false;
    lastButtonState[i] = false;
    lastDebounceTime[i] = 0;
  }

  for(int i = 0; i < NUM_LED_BOARDS; i++) {
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

  displayCount();
  
  // set digital pin to control as an output
  pinMode(13, OUTPUT);
  // set the data rate for the SoftwareSerial port
  BT.begin(115200);
  // Send test message to other device
  BT.println("Hello from Arduino");
}

/*
 This method will display the characters for the
 word "Arduino" one after the other on digit 0. 
 */
void writeArduinoOn7Segment() {
  int delaytime = 1000;
  
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

void loop() {
  for(int i = 0; i < NUM_BUTTONS; i++) {
    // read the current state of the pin reading into a local variable
    bool reading = (digitalRead(buttonPins[i]) == LOW ? true : false);
  
    // check to see if you just pressed the button
    // (i.e. the input went from LOW to HIGH), and you've waited long enough
    // since the last press to ignore any noise:
  
    // If the switch changed, due to noise or pressing:
    if (reading != lastButtonState[i]) {
      // reset the debouncing timer
      lastDebounceTime[i] = millis();
    }
  
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      // whatever the reading is at, it's been there for longer than the debounce
      // delay, so take it as the actual current state:
  
      // if the button state has changed:
      if (reading != buttonState[i]) {
        buttonState[i] = reading;
  
        // process the button press if the new state is pressed
        if (buttonState[i]) {
          // if this is an odd numbered button, decrement the count, otherwise increment the count
          if((i % 2) == 0) {
            count--;
          }
          else {
            count++;
          }

          displayCount();
          
          // transmit the new count over bluetooth
          BT.print("count = ");
          BT.println(count);
        }
      }
    }
  
    // save the reading. Next time through the loop, it'll be the lastButtonState:
    lastButtonState[i] = reading;
  }
}

//char a; // stores incoming character from other device
//void loop() 
//{
//  if (BT.available())
//  // if text arrived in from BT serial...
//  {
//    a=(BT.read());
//    if (a=='1')
//    {
//      digitalWrite(13, HIGH);
//      BT.println(" LED on");
//    }
//    if (a=='2')
//    {
//      digitalWrite(13, LOW);
//      BT.println(" LED off");
//    }
//    if (a=='?')
//    {
//      BT.println("Send '1' to turn LED on");
//      BT.println("Send '2' to turn LED on");
//    }   
//    // you can add more "if" statements with other characters to add more commands
//  }
//}
