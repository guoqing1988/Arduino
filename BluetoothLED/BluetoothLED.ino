#include <SoftwareSerial.h>

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

void setup()  
{
  // set digital pin to control as an output
  pinMode(13, OUTPUT);
  // set the data rate for the SoftwareSerial port
  BT.begin(115200);
  // Send test message to other device
  BT.println("Hello from Arduino");
}

char a; // stores incoming character from other device
void loop() 
{
  if (BT.available())
  // if text arrived in from BT serial...
  {
    a=(BT.read());
    if (a=='1')
    {
      digitalWrite(13, HIGH);
      BT.println(" LED on");
    }
    if (a=='2')
    {
      digitalWrite(13, LOW);
      BT.println(" LED off");
    }
    if (a=='?')
    {
      BT.println("Send '1' to turn LED on");
      BT.println("Send '2' to turn LED on");
    }   
    // you can add more "if" statements with other characters to add more commands
  }
}
