#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>

// define the LCD display and associated variables
#define LCD_RS  2
#define LCD_E   3
#define LCD_D4  4
#define LCD_D5  5
#define LCD_D6  6
#define LCD_D7  7
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// define the SD card reader and associated variables
#define SD_CD   8
#define SD_CS  53
#define SD_FILENAME "PASSWORD.TXT"
#define SD_PASSWORD "WAR GAMES"
enum SDState {
  SDSTATE_MISSING,
  SDSTATE_INIT_ERROR,
  SDSTATE_FILE_ERROR,
  SDSTATE_BAD_PASSWORD,
  SDSTATE_GOOD_PASSWORD
};
  
// define and PIEZO buzzer and associated variables
#define PIEZO_PIN 13
#define PIEZO_TONE 349

#define DOT_TIME  200
#define DASH_TIME 600
#define CODE_BREAK_TIME (DOT_TIME / 2)
#define LETTER_BREAK_TIME (DASH_TIME * 4)
#define WORD_BREAK_TIME (DASH_TIME * 8)
#define MESSAGE_BREAK_TIME (DASH_TIME * 16)

enum Morse {
  DOT,
  DASH,
  LETTER_BREAK,
  WORD_BREAK,
  MESSAGE_BREAK
};

#define CODES_PER_NUMBER 5
Morse numberCodes[][CODES_PER_NUMBER] = {
  { DASH, DASH, DASH, DASH, DASH }, // 0
  {  DOT, DASH, DASH, DASH, DASH }, // 1
  {  DOT,  DOT, DASH, DASH, DASH }, // 2
  {  DOT,  DOT,  DOT, DASH, DASH }, // 3
  {  DOT,  DOT,  DOT,  DOT, DASH }, // 4
  {  DOT,  DOT,  DOT,  DOT,  DOT }, // 5
  { DASH,  DOT,  DOT,  DOT,  DOT }, // 6
  { DASH, DASH,  DOT,  DOT,  DOT }, // 7
  { DASH, DASH, DASH,  DOT,  DOT }, // 8
  { DASH, DASH, DASH, DASH,  DOT }  // 9
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // enable the pullup resistor on the CD pin of the SD card
  pinMode(SD_CD, INPUT_PULLUP);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
}

void playCode(Morse code) {
  boolean piezoOn = true;
  unsigned int duration = 0;
  unsigned int delayTime = 0;

  switch(code) {
    case DOT:
      delayTime = DOT_TIME;
      break;
    case DASH:
      delayTime = DASH_TIME;
      break;
    case LETTER_BREAK:
      piezoOn = false;
      delayTime = LETTER_BREAK_TIME;
      break;
    case WORD_BREAK:
      piezoOn = false;
      delayTime = WORD_BREAK_TIME;
      break;
    case MESSAGE_BREAK:
      piezoOn = false;
      delayTime = MESSAGE_BREAK_TIME;
      break;
    default:
      piezoOn = false;
      break;
  }

  if(piezoOn) {
    tone(PIEZO_PIN, PIEZO_TONE);
  }
  else {
    noTone(PIEZO_PIN);
  }
  
  delay(delayTime);

  // if we just played a dot or a dash, turn the tone off before the next code
  if((code == DOT) || (code == DASH)) {
    noTone(PIEZO_PIN);
    delay(CODE_BREAK_TIME);
  }
}

void playNumberCode(unsigned int num) {
  if((num < 0) || (num > 9)) {
    return;
  }
  
  for(int i = 0; i < CODES_PER_NUMBER; i++) {
    playCode(numberCodes[num][i]);
  }
}

#define MAX_BUF_LEN 64

boolean passwordFound() {
  static SDState priorState = SDSTATE_MISSING;
  static String password = SD_PASSWORD;
  static boolean firstTime = true;
  int passwordLen = 0;
  int passwordIndex = 0;
  SDState newState = priorState;

  // if the password was found in the past, don't check again
  if(priorState == SDSTATE_GOOD_PASSWORD) {
    return true;
  }

  // check to see if the SD card is inserted
  if(digitalRead(SD_CD) == HIGH) {
    // if the card was just inserted, then read the password file
    if(priorState == SDSTATE_MISSING) {
      // delay for 0.5 seconds to give time for card to be fully inserted
      delay(500);

      // initialize the SD card
      if(SD.begin(SD_CS)) {
        // open the password file
        File passwordFile = SD.open(SD_FILENAME);
        if(passwordFile) {
          // find the first character in the file that does not match the password
          passwordLen = password.length();
          for(passwordIndex = 0;
              (passwordIndex < passwordLen) &&
              (passwordFile.available()) &&
              (passwordFile.read() == password.charAt(passwordIndex));
              passwordIndex++);
      
          // close the file
          passwordFile.close();
  
          // if the loop reached the end of the password string, then
          // the password was verified, otherwise, the incorrect password
          // was supplied
          if(passwordIndex >= passwordLen) {
            newState = SDSTATE_GOOD_PASSWORD;
          }
          else {
            newState = SDSTATE_BAD_PASSWORD;
          }
        } else {
          // if the file didn't open, set the new state to a file error
          newState = SDSTATE_FILE_ERROR;
        }
      }
      else {
        // if the SD card didn't initialize, set the new state to an init error
        newState = SDSTATE_INIT_ERROR;
      }
    }
  }
  else {
    newState = SDSTATE_MISSING;
  }

  if((newState != priorState) || (firstTime)) {
    switch(newState) {
      case SDSTATE_MISSING:
        // Print a message to the LCD.
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("     LOCKED     ");
        lcd.setCursor(0, 1);
        lcd.print("   NO SD CARD   ");
        break;
      case SDSTATE_INIT_ERROR:
        // Print a message to the LCD.
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("     LOCKED     ");
        lcd.setCursor(0, 1);
        lcd.print("SD ERROR: INIT  ");
        break;
      case SDSTATE_FILE_ERROR:
        // Print a message to the LCD.
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("     LOCKED     ");
        lcd.setCursor(0, 1);
        lcd.print("SD ERROR: FILE  ");
        break;
      case SDSTATE_BAD_PASSWORD:
        // Print a message to the LCD.
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("     LOCKED     ");
        lcd.setCursor(0, 1);
        lcd.print("SD: BAD PASSWORD");
        break;
      case SDSTATE_GOOD_PASSWORD:
        // Print a message to the LCD.
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("****UNLOCKED****");
        lcd.setCursor(0, 1);
        lcd.print("  ...Listen...  ");
        break;
    }
  }

  priorState = newState;
  firstTime = false;

  return (priorState == SDSTATE_GOOD_PASSWORD);
}

void loop() {
  if(passwordFound()) {
    playNumberCode(9);
    playCode(LETTER_BREAK);
  
    playNumberCode(2);
    playCode(LETTER_BREAK);
  
    playNumberCode(4);
    playCode(LETTER_BREAK);
  
    playNumberCode(7);
    playCode(MESSAGE_BREAK);
  }
}
