#include <Adafruit_CircuitPlayground.h>

#define XTILT_THRESHOLD -5

boolean interruptActive = false;

void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
}

boolean checkInterrupt() {
  float x, y, z;

  if(interruptActive) {
    return true;
  }
  
  x = CircuitPlayground.motionX();
  y = CircuitPlayground.motionY();
  z = CircuitPlayground.motionZ();

/*
  Serial.print(x);
  Serial.print(" ");
  Serial.print(y);
  Serial.print(" ");
  Serial.println(z);
*/

  return ((x > XTILT_THRESHOLD) && (z > 0));
}

void blinkLEDs(uint8_t r, uint8_t g, uint8_t b, int onTime, int offTime) {
  int i;
  
  for(i = 0; i < 10; i++) {
    CircuitPlayground.setPixelColor(i, r, g, b);
  }
  delay(onTime);
  CircuitPlayground.clearPixels();
  delay(offTime);
}

void spinLEDs() {
  int i;

  for(i = 4; i >= 0; i--) {
    if(checkInterrupt()) {
      return;
    }    
    CircuitPlayground.clearPixels();
    CircuitPlayground.setPixelColor(i, 0, 128, 0);
    CircuitPlayground.setPixelColor((i+5)%10, 0, 128, 0);
    delay(100);
  }
}

void loop() {
  int i;
  
  if(checkInterrupt()) {
    for(i = 750; i > 30; i -= 60) {
      blinkLEDs(64, 0, 0, 100, i);
    }
    for(i = 0; i < 10; i++) {      
      blinkLEDs(64, 0, 0, 100, 30);
    }
    blinkLEDs(255, 0, 0, 1000, 50);
    blinkLEDs(255, 255, 255, 500, 0);
    delay(2000);
    interruptActive = false;
  }

  spinLEDs();
}
