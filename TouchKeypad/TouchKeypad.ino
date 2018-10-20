#include <UTouch.h>
#include <UTouchCD.h>
#include <UTFT.h>

extern uint8_t SmallFont[];            //Declares Font
extern uint8_t BigFont[];
UTFT myGLCD(SSD1289,38,39,40,41);    //(Model,RS,WR,CS,RST)
UTouch myTouch ( 6, 5, 4, 3, 2);

void setup() {
  Serial.begin(9600);
  myTouch.InitTouch(0);
  myTouch.setPrecision(PREC_HI);
  myGLCD.InitLCD(0);
  myGLCD.clrScr();                    //clear screen
  myGLCD.setFont(SmallFont);
  myGLCD.fillScr(VGA_BLACK);          //fill screen with black
  myGLCD.setColor(VGA_RED);
  myGLCD.setBackColor(VGA_RED);
  delay(1000);
  drawWelcome_msg();
  delay(5000);
  drawButtons();
  Serial.println("SYSTEM STARTED");
}

void loop() {
  int x,y;
  while (true)
  {
    if (myTouch.dataAvailable())
    {
      myTouch.read();
      x=myTouch.getX();
      y=myTouch.getY();
      
      if ((y>=41) && (y<=89))    //top row
      {
        if ((x>=21) && (x<=85))  //button 1
        {
          Serial.println("1");
        }
        if ((x>=87) && (x<=152))  //button 2
        {
          Serial.println("2");
        }
        if ((x>=154) && (x<=218))  //button 3
        {
          Serial.println("3");
        }
      }
      if ((y>=91) && (y<=139))    //middle row
      {
        if ((x>=21) && (x<=85))  //button 4
        {
          Serial.println("4");
        }
        if ((x>=87) && (x<=152))  //button 5
        {
          Serial.println("5");
        }
        if ((x>=154) && (x<=218))  //button 6
        {
          Serial.println("6");
        }
      }
      if ((y>=141) && (y<=189))    //bottom row
      {
        if ((x>=21) && (x<=85))  //button 7
        {
          Serial.println("7");
        }
        if ((x>=87) && (x<=152))  //button 8
        {
          Serial.println("8");
        }
        if ((x>=154) && (x<=218))  //button 9
        {
          Serial.println("9");
        }
      }
      if ((y>=191) && (y<=238))    //bottom row
      {
        if ((x>=21) && (x<=85))  //clear button
        {
          Serial.println("CLEAR");
        }
        if ((x>=87) && (x<=152))  //button 0
        {
          Serial.println("0");
        }
        if ((x>=154) && (x<=218))  //enter button
        {
          Serial.println("ENTER");
        }
      }
    }
  }
}
void drawWelcome_msg(){
  myGLCD.fillRect(1,1,239,30);
  myGLCD.fillRect(1,289,239,319);
  myGLCD.fillRect(19,120,219,179);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.print("CAR SECURITY SYSTEM", CENTER, 10);
  myGLCD.print("SHANE CONCANNON - G00286539", CENTER, 299);
  myGLCD.print("*************************", CENTER, 120);
  myGLCD.print("*                       *", CENTER, 130);
  myGLCD.print("*    SYSTEM UNLOCKED    *", CENTER, 140);
  myGLCD.print("*   ENTER PIN TO LOCK   *", CENTER, 150);
  myGLCD.print("*                       *", CENTER, 160);
  myGLCD.print("*************************", CENTER, 169);
}
void drawButtons(){
  myGLCD.clrScr();
  myGLCD.setColor(VGA_RED);
  myGLCD.fillRect(1,1,239,30);
  myGLCD.fillRect(1,289,239,319);
  myGLCD.fillRoundRect(20,40,219,229);
  myGLCD.fillRoundRect(20,239,219,274);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.print("CAR SECURITY SYSTEM", CENTER, 10);
  myGLCD.print("SHANE CONCANNON - G00286539", CENTER, 299);
  myGLCD.drawRoundRect(20,40,219,229);
  myGLCD.drawRoundRect(20,239,219,274);
  myGLCD.print("  ___   ___  ___  ___  ", CENTER, 255);
  myGLCD.drawLine(86,40,86,229);
  myGLCD.drawLine(153,40,153,229);
  myGLCD.drawLine(20,90,219,90);
  myGLCD.drawLine(20,140,219,140);
  myGLCD.drawLine(20,190,219,190);
  myGLCD.print("CLEAR", 35, 204);
  myGLCD.print("ENTER", 167, 204);
  myGLCD.setFont(BigFont);
  myGLCD.printNumI(0,112,202);
  myGLCD.printNumI(1,45,57);
  myGLCD.printNumI(2,112,57);
  myGLCD.printNumI(3,178,57);
  myGLCD.printNumI(4,45,107);
  myGLCD.printNumI(5,112,107);
  myGLCD.printNumI(6,178,107);
  myGLCD.printNumI(7,45,157);
  myGLCD.printNumI(8,112,157);
  myGLCD.printNumI(9,178,157);
}

