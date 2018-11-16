/*
  ServoPuzzle

  This puzzle will set a servo to the zero position upon initialization,
  and then toggle the servo between 180 and 0 each time a button is pressed.
  The button should be wired between ground and the signal pin
  without a resistor because the pullup resistor is used on the arduino.
*/

#include <Servo.h>

#define PIN_BUTTON  2
#define PIN_SERVO   9

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

bool open = false;    // variable to store the servo position

void updateServoPosition() {
  myservo.write((open) ? 180 : 0);
}

void setup() {
  //configure pin 2 as an input for the button and enable the internal pull-up resistor
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  // attach the servo to pin 9
  myservo.attach(PIN_SERVO);
  
  open = false;
  updateServoPosition();
}

void loop() {
  static bool priorButtonPressed = false;
  
  // Check to see if the button is pressed
  // Keep in mind the pull-up means the pushbutton's logic is inverted. It goes
  // HIGH when it's open, and LOW when it's pressed.
  bool buttonPressed = (digitalRead(PIN_BUTTON) == LOW);

  // if this is the rising edge of a button press, toggle the open state
  // and update the servo position. Then delay for 2 seconds for debounce
  if(buttonPressed && !priorButtonPressed) {
    open = !open;
    updateServoPosition();

    delay(2000);
  }

  priorButtonPressed = buttonPressed;
}
