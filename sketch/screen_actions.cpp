#include "screen_actions.h"
#include <LiquidCrystal.h>

extern LiquidCrystal lcd;
static uint8_t flagsScreenAction = FLAG_NO_SCREEN_ACTION;

static uint8_t place = PLACE_ONES;

/**
 * Checks if a screen action flag is set.
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param FLAG The flag to check for.
 * 
 * @return bool True if the flag is set, false otherwise.
 */
bool checkScreenActionFlag(const uint8_t FLAG)
{
  if( (flagsScreenAction & FLAG) == FLAG )
  {
   return true; 
  }
  return false;
}

/**
 * Gets the screen action flags.
 * 
 * @author Miguel (7/10/2015)
 * 
 * @return uint8_t The screen action flags.
 */
uint8_t getScreenActionFlags()
{
  return flagsScreenAction;
}

/**
 * Sets a screen action flag.
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param FLAG The flag to set. 
 */
void setScreenActionFlag(const uint8_t FLAG)
{
  flagsScreenAction |= FLAG;
}

/**
 * Clears a screen action flag.
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param FLAG The flag to clear.
 */
void clearScreenActionFlag(const uint8_t FLAG)
{
  flagsScreenAction &= ~ FLAG ;
}

/**
 * Clears all screen action flags.
 * 
 * @author Miguel (7/10/2015)
 */
void clearScreenActionFlags()
{
    flagsScreenAction = 0x00;
}

/**
 * Sets the factor variable that is displayed in the LCD.
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param value The value to set the flag to.
 * @param factor A pointer to the variable which holds the 
 *               factor value.
 */
void setFactor(float value, float *factor)
{

  *factor = roundf( (*factor) * 1000)/1000;
  
  if( (value>=0.980) && (value<=1.500f) )
  {
    *factor = value;
    
  }
  else if(value<0.980)
  {
    *factor = 0.980f;
  }
  else if(value>1.500f)
  {
    *factor = 1.500f;
  }
}

/**
 * Increments the factor's current figure by one.
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param f A pointer to the variable which holds the factor 
 *          value.
 */
void incrementFactor(float * f)
{
  float change = 0.000; // how much the factor will be changed by.
  uint8_t place = getPlace(); // get the place in the figure to know which figure to increment.
  switch(place)
  {
    case PLACE_ONES: // The user was currently selecting the ones place, so increment that place figure by one.
      change = 1.000f;
    break;
    case PLACE_TENTHS:
      change = 0.100f;
    break;
    case PLACE_HUNDREDTHS:
      change = 0.010f;
    break;
    case PLACE_THOUSANDTHS:
      change = 0.001f;
    break;
    default:
    break;
  }
  
  setFactor(*f+change,f);

  char factorString[6] = {'\0'};
  
  // Can't sprintf a float so let's seperate it.
  int d1 = *f; // get the number to the left of the decimal
  int d2 = trunc(((*f)-d1)*1000); // get the number to the right of the decimal
  sprintf(factorString,"%d.%.3d",d1,d2);
  lcd.setCursor(8,1);
  lcd.print(factorString);
  setFactorCursor(getPlace());
    
}

/**
 * Decrements a factor's place value by one.
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param factor The variable which holds the factor value.
 */
void decrementFactor(float * factor)
{
  float change = 0.000;
  uint8_t place = getPlace();
  switch(place)
  {
    case PLACE_ONES:
      change = -1.000f;
    break;
    case PLACE_TENTHS:
      change = -0.100f;
    break;
    case PLACE_HUNDREDTHS:
      change = -0.010f;
    break;
    case PLACE_THOUSANDTHS:
      change = -0.001f;
    break;
    default:
    break;
  }
  
  setFactor(*factor+change,factor);
  char factorString[6] = {'\0'};
  
  // Can't sprintf a float so let's seperate it
  int d1 = *factor; // get the number to the left of the decimal
  int d2 = trunc(((*factor)-d1)*1000); // get the number to the right of the decimal
  sprintf(factorString,"%d.%.3d",d1,d2);
  lcd.setCursor(8,1);
  lcd.print(factorString);
  setFactorCursor(getPlace());  
}

/**
 * Sets the LCD cursor on the desired place i.e. ones, tenths, 
 * hundreths, thousanths. 
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param place The place where the cursor should be 
 *              placed/displayed/set.
 */
void setFactorCursor(uint8_t place)
{
  switch(place)
  {
    case PLACE_ONES: // pu thte cursor in the one's place.
      lcd.setCursor(8,1);
    break;
    case PLACE_TENTHS:
      lcd.setCursor(10,1);
    break;
    case PLACE_HUNDREDTHS:
      lcd.setCursor(11,1);
    break;
    case PLACE_THOUSANDTHS:
      lcd.setCursor(12,1);
    break;
    default:
    break;
  }
}

/**
 * Moves the place the user is selecting to the right. Note: 
 * does not move cursor in the LCD. 
 * 
 * @author Miguel (7/10/2015)
 */
void movePlaceRight()
{
  setPlace(place+1);
}

/**
 * Moves the place the user is selecting to the left. Note: does
 * not move the cursor in the LCD.
 * 
 * @author Miguel (7/10/2015)
 */
void movePlaceLeft()
{
  setPlace(place-1);
}

/**
 * Sets the place variable.
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param p The place i.e. ones, thousandths.
 */
void setPlace(const uint8_t p)
{
  if( (p>=PLACE_ONES) && (p<=PLACE_THOUSANDTHS))
  {
    place = p;
  }
}

/**
 * Get the current place.
 * 
 * @author Miguel (7/10/2015)
 * 
 * @return uint8_t 
 */
uint8_t getPlace()
{
 return place; 
}

/**
 * Resets the place variable to the ones place.
 * 
 * @author Miguel (7/10/2015)
 */
void resetPlace()
{
  place = PLACE_ONES;
}

/**
 * Updates the current reading screen with a new current reading 
 * value. 
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param reading 
 */
void updateCRMainScreen(float reading)
{
 lcd.setCursor(4,1);
 lcd.print(reading);
}

/**
 * Updates the current reading screen's current reading location 
 * with a string. 
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param s 
 */
void updateCRMainScreen(const char * s)
{
 lcd.setCursor(4,1);
 lcd.print(s);
}

/**
 * Updates the remaining calibration time on the LCD.
 * 
 * @author Miguel (7/10/2015)
 * 
 * @param timeRemaining The time remaining for calibration.
 */
void updateFirstCalibrationRemaningTime(char * timeRemaining)
{
  lcd.setCursor(12,1);
  lcd.print(timeRemaining);
}
