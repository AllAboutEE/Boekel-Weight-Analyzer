#include "screen_draw.h"
#include <LiquidCrystal.h>

extern LiquidCrystal lcd;

static uint8_t flagsScreenDraw = FLAG_DRAW_BOOT_SCREEN;

/**
 * Checks if a screen flag is set. Can be (and is) used to know 
 * which screen should be displayed. 
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param FLAG The unique screen flag/id
 * 
 * @return bool True if a flag is set false otherwise
 */
bool checkScreenDrawFlag(const uint8_t FLAG)
{
  if(flagsScreenDraw == FLAG )
  {
   return true; 
  }
  return false;
}

/**
 * Obtains/Gets the screen flag values.
 * 
 * @author Miguel (7/10/2015)
 * 
 * @return uint8_t The screen flags.
 */
uint8_t getScreenDrawFlags()
{
  return flagsScreenDraw;
}

/**
 * Sets a screen flag.
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param FLAG The screen flag to set
 */
void setScreenDrawFlag(const uint8_t FLAG)
{
  flagsScreenDraw = FLAG; // only one screen can be set at a time.
}

/**
 * Clears all the screen flags.
 * 
 * @author Miguel (7/10/2015)
 */
void clearScreenDrawFlags()
{
    flagsScreenDraw = 0x00;
}

/**
 * Draws the boot screen for 5 seconds.
 * 
 * @author Miguel (7/10/2015)
 */
void drawBootScreen()
{
  // Dispaly Boot Screen for 5 seconds
  lcd.setCursor(0,0);
  lcd.print("Weight Analyzer");
  lcd.setCursor(0,1);
  lcd.print("Version 1.0"); // TODO define software version at top of file and pass to this line.
  delay(5000); 
}

/**
 * Draws the main screen.
 * 
 * @author Miguel (7/10/2015)
 */
void drawMainScreen()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Weight Analyzer");
  lcd.setCursor(0,1);
  lcd.print("CR: "); // Current Reading
  
}

/**
 * Draws the screen where the user is asked if they'll like to 
 * proceed with the calibration. 
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param selection The selected option.
 */
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

/**
 * Draws the screen where the user is told that calibration is 
 * in progress. 
 * 
 * @author Miguel (7/10/2015)
 */
void drawCalibrationInProgessScreen()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calibrating!");
  lcd.setCursor(0,1);
  lcd.print("Don't touch");
}

/**
 * Draws the screen where the user can enter the factor.
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param factor 
 */
void drawEnterFactorOneScreen(char * factor)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Use U/D/L/R");
  lcd.setCursor(0,1);
  lcd.print("Factor: ");
  lcd.print(factor);
  lcd.setCursor(8,1);
  lcd.cursor();
  lcd.blink();
}

/**
 * Draws the screen where upon pressing the select button the 
 * user will initiate the calibration. 
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param string The string to display in the first line of the 
 *               LCD.
 */
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

