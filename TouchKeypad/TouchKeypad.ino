/*
#include <UTouch.h>
#include <UTouchCD.h>
#include <UTFT.h>
*/

#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <stdint.h>
#include "TouchScreen.h"

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

/*
extern uint8_t SmallFont[];            //Declares Font
extern uint8_t BigFont[];
UTFT myGLCD(SSD1289,38,39,40,41);    //(Model,RS,WR,CS,RST)
UTouch myTouch ( 6, 5, 4, 3, 2);
*/

void setup() {
  Serial.begin(9600);

  tft.reset();

  uint16_t identifier = tft.readID();
identifier=0x9341;
  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    return;
  }

  tft.begin(identifier);

/*
  myTouch.InitTouch(0);
  myTouch.setPrecision(PREC_HI);
  myGLCD.InitLCD(0);
  myGLCD.clrScr();                    //clear screen
  myGLCD.setFont(SmallFont);
  myGLCD.fillScr(VGA_BLACK);          //fill screen with black
*/
  tft.fillScreen(BLACK);

/*
  myGLCD.setColor(VGA_RED);
  myGLCD.setBackColor(VGA_RED);
  delay(1000);
  drawWelcome_msg();
  delay(5000);
*/

  drawButtons();

  Serial.println("SYSTEM STARTED");
}

void loop() {
  int x,y;

  // a point object holds x y and z coordinates
  TSPoint p = ts.getPoint();
  
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  // was previously p.z > ts.pressureThreshhold
  if (p.z > 0) {    
    Serial.print("PX = "); Serial.print(p.x);
    Serial.print("\tPY = "); Serial.print(p.y);
    Serial.print("\tPressure = "); Serial.print(p.z);

    x = map(p.y, 920, 110, 0, 239);
    y = map(p.x, 110, 960, 0, 319);
      
    Serial.print("\tX = "); Serial.print(x);
    Serial.print("\tY = "); Serial.println(y);

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

  delay(100);

/*
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
*/
}

/*
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
*/

void drawButtons(){
/*
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
*/

  tft.fillScreen(BLACK);
/*
  myGLCD.setColor(VGA_RED);
*/
  tft.fillRect(1,1,239,30,RED);
  tft.fillRect(1,289,239,319,RED);
  tft.fillRoundRect(20,40,(219-20+1),(229-40+1), 10, RED);
  tft.fillRoundRect(20,239,(219-20+1),(274-239+1), 10, RED);

/*
  myGLCD.setColor(VGA_WHITE);
  myGLCD.print("CAR SECURITY SYSTEM", CENTER, 10);
  myGLCD.print("SHANE CONCANNON - G00286539", CENTER, 299);
*/
  tft.setCursor(7, 8);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("CAR SECURITY SYSTEM");
  
  tft.drawRoundRect(20,40,(219-20+1),(229-40+1), 10, WHITE);
  tft.drawRoundRect(20,239,(219-20+1),(274-239+1), 10, WHITE);
/*
  myGLCD.print("  ___   ___  ___  ___  ", CENTER, 255);
*/
  tft.drawLine(86,40,86,229, WHITE);
  tft.drawLine(153,40,153,229, WHITE);
  tft.drawLine(20,90,219,90, WHITE);
  tft.drawLine(20,140,219,140, WHITE);
  tft.drawLine(20,190,219,190, WHITE);
/*
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
*/
}


