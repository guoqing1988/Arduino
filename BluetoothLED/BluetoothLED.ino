#include <SoftwareSerial.h>

// Was able to get this to work on an Arduino UNO
// For some reason, the same setup on a Mega 2560 did not work
int bluetoothTX = 2; // TX-0 pin of bluetooth on Arduino D2
int bluetoothRX = 3; // RX-1 pin of bluetooth on Arduino D3

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
