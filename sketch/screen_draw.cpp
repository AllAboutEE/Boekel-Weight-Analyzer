#include "screen_draw.h"
#include <LiquidCrystal.h>

extern LiquidCrystal lcd;

static uint8_t flagsScreenDraw = FLAG_DRAW_BOOT_SCREEN;

bool checkScreenDrawFlag(const uint8_t FLAG)
{
  if(flagsScreenDraw == FLAG )
  {
   return true; 
  }
  return false;
}

uint8_t getScreenDrawFlags()
{
  return flagsScreenDraw;
}

uint8_t setScreenDrawFlag(const uint8_t FLAG)
{
  flagsScreenDraw = FLAG; // only one screen can be set at a time.
}

uint8_t clearScreenDrawFlag(const uint8_t FLAG)
{
  flagsScreenDraw &= ~ FLAG ;
}

void clearScreenDrawFlags()
{
    flagsScreenDraw = 0x00;
}

void drawBootScreen()
{
  // Dispaly Boot Screen for 5 seconds
  lcd.setCursor(0,0);
  lcd.print("Weight Analyzer");
  lcd.setCursor(0,1);
  lcd.print("Version 1.0"); // TODO define software version at top of file and pass to this line.
  delay(5000); 
}

void drawMainScreen()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Weight Analyzer");
  lcd.setCursor(0,1);
  lcd.print("CR: "); // Current Reading
  
}

void drawConfirmCalibrationScreen(uint8_t selection)
{
  lcd.clear();
  lcd.setCursor(0,0);
  
  if(selection == 0)
  {
    lcd.print("Calibrate WA *");
    lcd.setCursor(0,1);
    lcd.print("Exit");
  }
  else if(selection == 1)
  {
    lcd.print("Calibrate WA");
    lcd.setCursor(0,1);
    lcd.print("Exit *");  
  }
}

void drawCalibrationInProgessScreen()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calibrating!");
  lcd.setCursor(0,1);
  lcd.print("Don't touch");
}

void drawEnterFactorOneScreen()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Use U/D/L/R");
  lcd.setCursor(0,1);
  lcd.print("Factor: 1.000");
  lcd.setCursor(8,1);
  lcd.cursor();
  lcd.blink();
}

void drawCalibrationPromptScreen(const char * string)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(string);
  lcd.setCursor(0,1);
  lcd.print("Press \"Select\"");
  lcd.noCursor();
  lcd.noBlink();
}

