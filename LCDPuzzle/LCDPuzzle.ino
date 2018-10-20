// include the library code:
#include <LiquidCrystal.h>
#include <Servo.h>

// define the Servo
#define SERVO_PIN 10
Servo lockServo;

// define the LCD display and associated variables
#define LCD_RS  2
#define LCD_E   3
#define LCD_D4  4
#define LCD_D5  5
#define LCD_D6  6
#define LCD_D7  7
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// define the SD card reader and associated variables
enum SDState {
  MISSING,
  BAD_PASSWORD,
  GOOD_PASSWORD
};
SDState currentSDState = MISSING;
  
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

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  lockServo.attach(SERVO_PIN);
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

  Serial.print("On = ");
  Serial.print(piezoOn);
  Serial.print(" Delay = ");
  Serial.println(delayTime);
  
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

void loop() {
  switch(currentSDState) {
    case MISSING:
      lockServo.write(0);
      
      // Print a message to the LCD.
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("     LOCKED     ");
      lcd.setCursor(0, 1);
      lcd.print("   NO SD CARD   ");

      currentSDState = BAD_PASSWORD;
      break;
    case BAD_PASSWORD:
      lockServo.write(0);

      // Print a message to the LCD.
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("     LOCKED     ");
      lcd.setCursor(0, 1);
      lcd.print("SD: BAD PASSWORD");

      currentSDState = GOOD_PASSWORD;
      break;
    case GOOD_PASSWORD:
      lockServo.write(90);

      // Print a message to the LCD.
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("****UNLOCKED****");
      lcd.setCursor(0, 1);
      lcd.print("  ...Listen...  ");

      currentSDState = MISSING;
      break;
  }

  playNumberCode(9);
  playCode(LETTER_BREAK);

  playNumberCode(2);
  playCode(LETTER_BREAK);

  playNumberCode(4);
  playCode(LETTER_BREAK);

  playNumberCode(7);
  playCode(MESSAGE_BREAK);
}
